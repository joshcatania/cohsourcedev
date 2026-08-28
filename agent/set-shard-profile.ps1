[CmdletBinding()]
param(
    [ValidateSet('FastDev', 'Full')]
    [string]$Profile,
    [switch]$Status,
    [ValidateSet('On', 'Off')]
    [string]$TsrMode = 'Off',
    [switch]$DisableChatServer,
    [switch]$Json
)

$ErrorActionPreference = 'Stop'
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$configPath = Join-Path $repoRoot 'bin\data\server\db\servers.cfg'
$loadBalancePath = Join-Path $repoRoot 'bin\data\server\db\loadBalanceShardSpecific.cfg'
$baselinePath = Join-Path $PSScriptRoot 'shard-profile-baseline.json'
$stateDir = Join-Path $PSScriptRoot 'work'
$statePath = Join-Path $stateDir 'shard-profile-state.json'

if (-not $Status -and -not $Profile) { throw 'Choose -Profile FastDev, -Profile Full, or -Status.' }
if ($Status -and $Profile) { throw 'Choose either -Status or -Profile, not both.' }
if (-not (Test-Path -LiteralPath $configPath)) { throw "DbServer config not found: $configPath" }
if (-not (Test-Path -LiteralPath $loadBalancePath)) { throw "Shard load-balance config not found: $loadBalancePath" }
if (-not (Test-Path -LiteralPath $baselinePath)) { throw "Tracked shard-profile baseline not found: $baselinePath" }
$baseline = Get-Content -Raw -LiteralPath $baselinePath | ConvertFrom-Json
if ($baseline.version -ne 1) { throw "Unsupported shard-profile baseline version in $baselinePath." }

function Get-Text([string]$Path) { return [System.IO.File]::ReadAllText($Path) }
function Get-Sha256([string]$Path) {
    $sha = [System.Security.Cryptography.SHA256]::Create()
    try { return ([BitConverter]::ToString($sha.ComputeHash([System.IO.File]::ReadAllBytes($Path))) -replace '-', '').ToLowerInvariant() }
    finally { $sha.Dispose() }
}
function Write-ExactText([string]$Path, [string]$Text) {
    $utf8 = New-Object System.Text.UTF8Encoding($false)
    # Regex replacements can otherwise drop the CR from a matched CRLF line.
    # Keep the repository's canonical Windows config bytes stable across runs.
    $Text = [regex]::Replace($Text, "`r?`n", "`r`n")
    [System.IO.File]::WriteAllText($Path, $Text, $utf8)
}
function Get-LineEnding([string]$Text) { if ($Text.Contains("`r`n")) { return "`r`n" } return "`n" }

function Set-Directive([string]$Text, [string]$Name, [int]$Value) {
    $pattern = "(?im)^([ \t]*)$([regex]::Escape($Name))[ \t]+(?:[0-9]+)([ \t]*(?://[^\r\n]*)?)\r?$"
    $matches = [regex]::Matches($Text, $pattern)
    if ($matches.Count -ne 1) { throw "Refusing to change servers.cfg: expected exactly one active $Name directive, found $($matches.Count)." }
    $replacement = '$1' + $Name + ' ' + $Value + '$2'
    return [regex]::Replace($Text, $pattern, $replacement, 1)
}

function Get-ServerBlock([string]$Text, [string]$AppName) {
    $lineEnding = Get-LineEnding $Text
    $lines = [System.Collections.Generic.List[string]]@($Text -split "`r?`n", -1)
    $appIndexes = @()
    for ($i = 0; $i -lt $lines.Count; $i++) {
        if ($lines[$i] -match ('^\s*(//\s*)?AppName\s+"' + [regex]::Escape($AppName) + '"\s*$')) { $appIndexes += $i }
    }
    if ($appIndexes.Count -ne 1) { throw "Refusing to change loadBalanceShardSpecific.cfg: expected exactly one $AppName AppName line, found $($appIndexes.Count)." }
    $appIndex = $appIndexes[0]
    $start = $null
    for ($i = $appIndex; $i -ge 0; $i--) {
        if ($lines[$i] -match '^\s*(//\s*)?Server\s*$') { $start = $i; break }
    }
    $end = $null
    for ($i = $appIndex; $i -lt $lines.Count; $i++) {
        if ($lines[$i] -match '^\s*(//\s*)?End\s*$') { $end = $i; break }
    }
    if ($null -eq $start -or $null -eq $end) { throw "Refusing to change loadBalanceShardSpecific.cfg: malformed $AppName Server block." }
    $isCommented = $lines[$start] -match '^\s*//'
    return [pscustomobject]@{ lines = $lines; start = $start; end = $end; active = -not $isCommented; lineEnding = $lineEnding }
}

function Set-ServerBlock([string]$Text, [string]$AppName, [bool]$Enabled) {
    $block = Get-ServerBlock $Text $AppName
    if ($block.active -eq $Enabled) { return $Text }
    for ($i = $block.start; $i -le $block.end; $i++) {
        if ($Enabled) {
            $block.lines[$i] = [regex]::Replace($block.lines[$i], '^([ \t]*)//', '$1', 1)
        } else {
            $block.lines[$i] = '//' + $block.lines[$i]
        }
    }
    return ($block.lines -join $block.lineEnding)
}

function Get-BlockActive([string]$Text, [string]$AppName) { return (Get-ServerBlock $Text $AppName).active }
function Apply-TextFixes([string]$Text, [string]$Profile) {
    foreach ($fix in @($baseline.loadBalanceTextFixes)) {
        $from = if ($Profile -eq 'Full') { $fix.fastDev } else { $fix.full }
        $to = if ($Profile -eq 'Full') { $fix.full } else { $fix.fastDev }
        if ($Text.Contains($from)) { $Text = $Text.Replace($from, $to) }
    }
    return $Text
}
function Get-DirectiveValue([string]$Text, [string]$Name) {
    $matches = [regex]::Matches($Text, "(?im)^\s*$([regex]::Escape($Name))\s+([0-9]+)\s*(?://.*)?$")
    if ($matches.Count -ne 1) { throw "Expected exactly one active $Name directive, found $($matches.Count)." }
    return [int]$matches[0].Groups[1].Value
}

function Get-ProfileClassification([string]$ConfigText, [string]$LoadText) {
    $full = ((Get-DirectiveValue $ConfigText 'NoStats') -eq 0 -and
        (Get-DirectiveValue $ConfigText 'UseLogServer') -eq 1 -and
        (Get-DirectiveValue $ConfigText 'DoNotLaunchBeaconMasterServer') -eq 0 -and
        (Get-DirectiveValue $ConfigText 'BeaconRequestServerCount') -eq 1 -and
        (Get-DirectiveValue $ConfigText 'BeaconClientCount') -eq 2 -and
        (Get-DirectiveValue $ConfigText 'DoNotLaunchBeaconClients') -eq 0 -and
        (Get-DirectiveValue $ConfigText 'DoNotLaunchMapServerTSR') -eq 0 -and
        (Get-DirectiveValue $LoadText 'RequestBeaconServerCount') -eq 1)
    foreach ($name in @('AccountServer','AuctionServer','ArenaServer','MissionServer','RaidServer','StatServer','TurnstileServer','LogServer','ChatServer')) {
        $full = $full -and (Get-BlockActive $LoadText $name)
    }
    if ($full) { return 'Full' }

    $fast = ((Get-DirectiveValue $ConfigText 'NoStats') -eq 1 -and
        (Get-DirectiveValue $ConfigText 'UseLogServer') -eq 0 -and
        (Get-DirectiveValue $ConfigText 'DoNotLaunchBeaconMasterServer') -eq 1 -and
        (Get-DirectiveValue $ConfigText 'BeaconRequestServerCount') -eq 0 -and
        (Get-DirectiveValue $ConfigText 'BeaconClientCount') -eq 0 -and
        (Get-DirectiveValue $ConfigText 'DoNotLaunchBeaconClients') -eq 1 -and
        (Get-DirectiveValue $LoadText 'RequestBeaconServerCount') -eq 0)
    foreach ($name in @('AccountServer','AuctionServer','ArenaServer','MissionServer','RaidServer','StatServer','TurnstileServer','LogServer')) {
        $fast = $fast -and -not (Get-BlockActive $LoadText $name)
    }
    if ($fast) { return 'FastDev' }
    # Repair FastDev states created before the load-balance beacon count was
    # included in the guarded transform.
    $legacyFast = ((Get-DirectiveValue $ConfigText 'NoStats') -eq 1 -and
        (Get-DirectiveValue $ConfigText 'UseLogServer') -eq 0 -and
        (Get-DirectiveValue $ConfigText 'DoNotLaunchBeaconMasterServer') -eq 1 -and
        (Get-DirectiveValue $ConfigText 'BeaconRequestServerCount') -eq 0 -and
        (Get-DirectiveValue $ConfigText 'BeaconClientCount') -eq 0 -and
        (Get-DirectiveValue $ConfigText 'DoNotLaunchBeaconClients') -eq 1)
    foreach ($name in @('AccountServer','AuctionServer','ArenaServer','MissionServer','RaidServer','StatServer','TurnstileServer','LogServer')) {
        $legacyFast = $legacyFast -and -not (Get-BlockActive $LoadText $name)
    }
    if ($legacyFast) { return 'FastDev' }
    return 'Unknown'
}

$beforeConfigText = Get-Text $configPath
$beforeLoadText = Get-Text $loadBalancePath
$beforeConfigSha = Get-Sha256 $configPath
$beforeLoadSha = Get-Sha256 $loadBalancePath
$beforeProfile = Get-ProfileClassification $beforeConfigText $beforeLoadText
$state = if (Test-Path -LiteralPath $statePath) { Get-Content -Raw -LiteralPath $statePath | ConvertFrom-Json } else { $null }
$baselineCurrent = (($beforeConfigSha -eq $baseline.configSha256.$beforeProfile) -and
    ($beforeLoadSha -eq $baseline.loadBalanceSha256.$beforeProfile))

if ($Status) {
    $statusResult = [pscustomobject]@{
        profile = $beforeProfile
        tsr = if ((Get-DirectiveValue $beforeConfigText 'DoNotLaunchMapServerTSR') -eq 1) { 'Off' } else { 'On' }
        chatServer = if (Get-BlockActive $beforeLoadText 'ChatServer') { 'Retained' } else { 'Disabled' }
        config = $configPath
        loadBalanceConfig = $loadBalancePath
        configSha256 = $beforeConfigSha
        loadBalanceSha256 = $beforeLoadSha
        guardedStatePresent = ($null -ne $state)
        statePath = $statePath
    }
    if ($Json) { $statusResult | ConvertTo-Json -Depth 6 } else {
        Write-Host ("Active shard profile: {0}" -f $statusResult.profile)
        Write-Host ("TSR: {0}; ChatServer: {1}" -f $statusResult.tsr, $statusResult.chatServer)
        Write-Host ("Guarded state: {0}" -f $(if ($state) { $statePath } else { 'not initialized' }))
    }
    exit 0
}

if ($Profile -eq 'Full' -and ($TsrMode -ne 'Off' -or $DisableChatServer)) { throw 'TSR and ChatServer options apply only to FastDev.' }

if ($null -ne $state) {
    $knownCurrent = (($beforeConfigSha -eq $state.fullConfigSha -and $beforeLoadSha -eq $state.fullLoadBalanceSha) -or
        ($beforeConfigSha -eq $state.fastConfigSha -and $beforeLoadSha -eq $state.fastLoadBalanceSha))
    if (-not $knownCurrent) {
        throw "Refusing profile change: configuration differs from the guarded Full/FastDev hashes in $statePath. Inspect manual edits before retrying."
    }
}
elseif (-not $baselineCurrent) {
    throw "Refusing profile change: no guarded state exists and the current $beforeProfile configuration does not match the tracked baseline. Inspect manual edits before retrying."
}

if ($Profile -eq $beforeProfile -and $Profile -eq 'Full') {
    $changed = $false
} elseif ($Profile -eq 'FastDev' -and $beforeProfile -eq 'FastDev' -and $null -ne $state -and
    $state.fastTsrMode -eq $TsrMode -and [bool]$state.fastChatDisabled -eq [bool]$DisableChatServer -and
    (Get-DirectiveValue $beforeLoadText 'RequestBeaconServerCount') -eq 0) {
    $changed = $false
} else {
    if ($Profile -eq 'FastDev' -and $beforeProfile -eq 'Unknown') { throw 'Refusing FastDev transform: current configuration is neither the known Full profile nor a guarded FastDev profile.' }
    if ($Profile -eq 'Full' -and $beforeProfile -eq 'Unknown') { throw 'Refusing Full restore: current configuration is neither the known Full profile nor a guarded FastDev profile.' }

    $afterConfigText = $beforeConfigText
    $afterLoadText = $beforeLoadText
    $fast = $Profile -eq 'FastDev'
    $afterConfigText = Set-Directive $afterConfigText 'NoStats' $(if ($fast) { 1 } else { 0 })
    $afterConfigText = Set-Directive $afterConfigText 'UseLogServer' $(if ($fast) { 0 } else { 1 })
    $afterConfigText = Set-Directive $afterConfigText 'DoNotLaunchBeaconMasterServer' $(if ($fast) { 1 } else { 0 })
    $afterConfigText = Set-Directive $afterConfigText 'BeaconRequestServerCount' $(if ($fast) { 0 } else { 1 })
    $afterConfigText = Set-Directive $afterConfigText 'BeaconClientCount' $(if ($fast) { 0 } else { 2 })
    $afterConfigText = Set-Directive $afterConfigText 'DoNotLaunchBeaconClients' $(if ($fast) { 1 } else { 0 })
    $afterConfigText = Set-Directive $afterConfigText 'DoNotLaunchMapServerTSR' $(if ($fast -and $TsrMode -eq 'Off') { 1 } else { 0 })
    $afterLoadText = Set-Directive $afterLoadText 'RequestBeaconServerCount' $(if ($fast) { 0 } else { 1 })
    foreach ($name in @('AccountServer','AuctionServer','ArenaServer','MissionServer','RaidServer','StatServer','TurnstileServer','LogServer')) {
        $afterLoadText = Set-ServerBlock $afterLoadText $name (-not $fast)
    }
    $afterLoadText = Set-ServerBlock $afterLoadText 'ChatServer' (-not ($fast -and $DisableChatServer))
    $afterLoadText = Apply-TextFixes $afterLoadText $Profile

    Write-ExactText $configPath $afterConfigText
    Write-ExactText $loadBalancePath $afterLoadText
    $afterClassification = Get-ProfileClassification (Get-Text $configPath) (Get-Text $loadBalancePath)
    if ($afterClassification -ne $Profile) { throw "Profile transform completed with unexpected classification: $afterClassification" }
    $changed = ($beforeConfigSha -ne (Get-Sha256 $configPath) -or $beforeLoadSha -ne (Get-Sha256 $loadBalancePath))
}

$afterConfigSha = Get-Sha256 $configPath
$afterLoadSha = Get-Sha256 $loadBalancePath
$afterConfigTextFinal = Get-Text $configPath
$afterLoadTextFinal = Get-Text $loadBalancePath
$afterProfile = Get-ProfileClassification $afterConfigTextFinal $afterLoadTextFinal

if ($Profile -eq 'FastDev') {
    New-Item -ItemType Directory -Force -Path $stateDir | Out-Null
    $fullConfigSha = if ($beforeProfile -eq 'Full' -and $beforeConfigSha -ne $afterConfigSha) { $beforeConfigSha } elseif ($state) { $state.fullConfigSha } else { $baseline.configSha256.Full }
    $fullLoadSha = if ($beforeProfile -eq 'Full' -and $beforeLoadSha -ne $afterLoadSha) { $beforeLoadSha } elseif ($state) { $state.fullLoadBalanceSha } else { $baseline.loadBalanceSha256.Full }
    $newState = [pscustomobject]@{
        fullConfigSha = $fullConfigSha
        fullLoadBalanceSha = $fullLoadSha
        fastConfigSha = $afterConfigSha
        fastLoadBalanceSha = $afterLoadSha
        fastTsrMode = $TsrMode
        fastChatDisabled = [bool]$DisableChatServer
        updatedAtUtc = [DateTime]::UtcNow.ToString('o')
    }
    $newState | ConvertTo-Json -Depth 5 | Set-Content -LiteralPath $statePath -Encoding UTF8
} elseif ($Profile -eq 'Full') {
    $expectedFullConfigSha = if ($state) { $state.fullConfigSha } else { $baseline.configSha256.Full }
    $expectedFullLoadSha = if ($state) { $state.fullLoadBalanceSha } else { $baseline.loadBalanceSha256.Full }
    if ($afterConfigSha -ne $expectedFullConfigSha -or $afterLoadSha -ne $expectedFullLoadSha) {
        throw "Full restore did not reproduce the guarded original configuration exactly. Refusing to continue; inspect $configPath and $loadBalancePath."
    }
    if (-not $state) {
        New-Item -ItemType Directory -Force -Path $stateDir | Out-Null
        [pscustomobject]@{
            fullConfigSha = $baseline.configSha256.Full
            fullLoadBalanceSha = $baseline.loadBalanceSha256.Full
            fastConfigSha = $baseline.configSha256.FastDev
            fastLoadBalanceSha = $baseline.loadBalanceSha256.FastDev
            fastTsrMode = 'Off'
            fastChatDisabled = $false
            updatedAtUtc = [DateTime]::UtcNow.ToString('o')
        } | ConvertTo-Json -Depth 5 | Set-Content -LiteralPath $statePath -Encoding UTF8
    }
}

$result = [pscustomobject]@{
    profile = $afterProfile
    changed = $changed
    restartRequired = $changed
    before = $beforeProfile
    after = $afterProfile
    tsr = if ((Get-DirectiveValue $afterConfigTextFinal 'DoNotLaunchMapServerTSR') -eq 1) { 'Off' } else { 'On' }
    chatServer = if (Get-BlockActive $afterLoadTextFinal 'ChatServer') { 'Retained' } else { 'Disabled' }
    config = $configPath
    loadBalanceConfig = $loadBalancePath
    statePath = $statePath
    note = 'FastDev transforms only guarded local directives and named Server blocks. The state hashes make later restores refuse unexpected/manual changes and Full restores the original bytes exactly.'
}
if ($Json) { $result | ConvertTo-Json -Depth 7 } else {
    Write-Host ("Profile: {0} -> {1}" -f $beforeProfile, $afterProfile)
    Write-Host ("TSR: {0}; ChatServer: {1}" -f $result.tsr, $result.chatServer)
    if ($changed) { Write-Host 'Profile configuration changed; restart ServerMonitor for it to take effect.' } else { Write-Host 'No profile change needed.' }
}
exit 0
