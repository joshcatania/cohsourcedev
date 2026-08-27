[CmdletBinding()]
param([string]$IncludePath = (Join-Path $PSScriptRoot 'webswing-animation\webswing.inc'))

$ErrorActionPreference = 'Stop'
if (-not (Test-Path -LiteralPath $IncludePath -PathType Leaf)) { throw "Missing include: $IncludePath" }

$moves = @{}
$current = $null
foreach ($line in Get-Content -LiteralPath $IncludePath) {
    if ($line -match '^\s*Move\s+(\S+)\s*$') {
        $current = [ordered]@{ Name=$Matches[1]; Member=@(); Interrupts=@(); Requires=@(); Flags=@(); NextMove=$null; Priority=0; Interpolate=0; Scale=1.0; Animation=$null; AnimStart=$null; AnimEnd=$null }
        $moves[$current.Name] = $current
        continue
    }
    if ($line -match '^\s*MEnd\s*$') { $current = $null; continue }
    if ($null -eq $current) { continue }
    if ($line -match '^\s*Priority\s+(\d+)\s*$') { $current.Priority = [int]$Matches[1] }
    elseif ($line -match '^\s*Scale\s+([0-9.]+)\s*$') { $current.Scale = [double]::Parse($Matches[1], [Globalization.CultureInfo]::InvariantCulture) }
    elseif ($line -match '^\s*Interpolate\s+(\d+)\s*$') { $current.Interpolate = [int]$Matches[1] }
    elseif ($line -match '^\s*Anim\s+(\S+)\s+(\d+)\s+(\d+)\s*$') { $current.Animation=$Matches[1]; $current.AnimStart=[int]$Matches[2]; $current.AnimEnd=[int]$Matches[3] }
    elseif ($line -match '^\s*NextMove\s+(\S+)\s*$') { $current.NextMove = $Matches[1] }
    elseif ($line -match '^\s*Flags\s+(.+)$') { $current.Flags = @($Matches[1] -split ',' | ForEach-Object { $_.Trim() }) }
    elseif ($line -match '^\s*(Member|Interrupts|Requires)\s+(.+)$') {
        $field = $Matches[1]
        $values = @([regex]::Matches($Matches[2], '"([^"]+)"') | ForEach-Object { $_.Groups[1].Value })
        if ($values.Count -eq 0 -and $field -eq 'Requires') { $values = @($Matches[2] -split ',' | ForEach-Object { $_.Trim().Trim('"') } | Where-Object { $_ }) }
        if ($values.Count -eq 0) { throw "Unreadable $field on $($current.Name)" }
        $current[$field] = $values
    }
}

function Get-Move([string]$Name) { if (-not $moves.ContainsKey($Name)) { throw "Missing move: $Name" }; $moves[$Name] }
$checkCount = 0
function Assert-True([string]$Label, [bool]$Value) { $script:checkCount++; if (-not $Value) { throw "FAIL: $Label" } }
function Assert-Contains([string]$Label, [string[]]$Values, [string]$Expected) { Assert-True $Label ($Values -contains $Expected) }
function Assert-Excludes([string]$Label, [string[]]$Values, [string]$Excluded) { Assert-True $Label (-not ($Values -contains $Excluded)) }

$standardMembers = @('<DEATHIRQ>','<HITIRQ>','<REACTIRQ>','<BLOCKIRQ>','<BLOCK>','<STUNMOVE>','<ATTACKIRQ>')
$standardInterrupts = @('<JUMPS>','<FALL>','<GROUNDMOVEALL>')
$phasedGroup = '<WEBSWING_V2_PHASED>'
$correctedGroup = '<WEBSWING_MALE_FULL_CORRECTED>'
$core = Get-Move 'WEBSWING_FULL_CORE'
Assert-True 'core uses accepted corrected asset and audited range' ($core.Animation -eq 'MALE/COHSOURCEDEV_RETARGET_RESTBASIS_SWING_FULL' -and $core.AnimStart -eq 6 -and $core.AnimEnd -eq 39)
Assert-True 'core is frame-scrubbed rather than free-running' ($core.Scale -eq 0)
Assert-Contains 'core requires private sync gate' $core.Requires 'WEBSWING_CORE_SYNC'
Assert-Contains 'core requires Sky-Assisted attached presentation state' $core.Requires 'WEBSWING_ATTACHED'
Assert-Contains 'core remains a cycling move for cursor bounds' $core.Flags 'Cycle'
Assert-Excludes 'core contains no terminal hold semantics' $core.Flags 'HoldLastFrame'
Assert-Excludes 'core excludes MOVEIRQ' $core.Member '<MOVEIRQ>'
Assert-Excludes 'core cannot steal authored shoot' $core.Interrupts '<WEBSWING_V2_SHOOT>'
Assert-Excludes 'core cannot steal authored retract/release' $core.Interrupts '<WEBSWING_V2_RETRACT>'
$coreText = Get-Content -Raw -LiteralPath $IncludePath
Assert-True 'core contains no frame-triggered sequencer FX' ($coreText -match '(?s)Move\s+WEBSWING_FULL_CORE\b(?<body>.*?)MEnd' -and $Matches.body -notmatch '(?m)^\s*Fx\b')
$launch = Get-Move 'WEBSWING_V2_GROUND_LAUNCH_START'
Assert-True 'accepted 62-frame first launch remains intact' ($launch.Animation -eq 'MALE/COHSOURCEDEV_WEBSWING_GROUND_LAUNCH_V2' -and $launch.AnimStart -eq 1 -and $launch.AnimEnd -eq 62)
Assert-Contains 'ground launch replaces phased choreography' $launch.Interrupts $phasedGroup
Assert-Excludes 'legacy retract cannot insert itself between ground launch and catch' (Get-Move 'WEBSWING_V2_RETRACT_START').Interrupts '<WEBSWING_V2_GROUND_LAUNCH>'

$events = @(
    [pscustomobject]@{ Prefix='WEBSWING_V2_REACH'; Bits=@('WEBSWING_SHOOT') },
    [pscustomobject]@{ Prefix='WEBSWING_V2_CATCH'; Bits=@('WEBSWING_ASCEND_MALE_ENTER','WEBSWING_RETRACT') },
    [pscustomobject]@{ Prefix='WEBSWING_V2_RELEASE'; Bits=@('WEBSWING_RETRACT') },
    [pscustomobject]@{ Prefix='WEBSWING_V2_AIRBORNE'; Bits=@('WEBSWING_SHOOT','WEBSWING_RETRACT') }
)
foreach ($event in $events) {
    foreach ($variant in @('A','B')) {
        $move = Get-Move "$($event.Prefix)_$variant"
        Assert-Contains "$($move.Name) phased member" $move.Member $phasedGroup
        foreach ($bit in $event.Bits) { Assert-Contains "$($move.Name) lifecycle requirement $bit" $move.Requires $bit }
        if ($variant -eq 'B') { Assert-Contains "$($move.Name) variant requirement" $move.Requires 'WEBSWING_VARIANT_B' }
        else { Assert-Excludes "$($move.Name) is the fallback variant" $move.Requires 'WEBSWING_VARIANT_B' }
        Assert-Contains "$($move.Name) replaces phased choreography" $move.Interrupts $phasedGroup
        Assert-Contains "$($move.Name) replaces physical phase" $move.Interrupts $correctedGroup
        Assert-Contains "$($move.Name) advances while active" $move.Flags 'Cycle'
        Assert-Contains "$($move.Name) holds its terminal pose instead of wrapping" $move.Flags 'HoldLastFrame'
        Assert-True "$($move.Name) moving window" ($move.Scale -gt 0 -and $move.AnimEnd -gt $move.AnimStart)
        Assert-Excludes "$($move.Name) excludes MOVEIRQ" $move.Member '<MOVEIRQ>'
        foreach ($group in $standardMembers) { Assert-Contains "$($move.Name) retains $group" $move.Member $group }
        foreach ($group in $standardInterrupts) { Assert-Contains "$($move.Name) interrupts $group" $move.Interrupts $group }
    }
}

$phaseMoves = @('WEBSWING_FULL_ATTACHED_START','WEBSWING_FULL_DESCEND_START','WEBSWING_FULL_BOTTOM_START','WEBSWING_FULL_ASCEND_START','WEBSWING_FULL_ATTACHED_ALT','WEBSWING_FULL_DESCEND_ALT','WEBSWING_FULL_BOTTOM_ALT','WEBSWING_FULL_ASCEND_ALT')
foreach ($name in $phaseMoves) {
    $move = Get-Move $name
    $variant = if ($name.EndsWith('_ALT')) { 'B' } else { 'A' }
    Assert-Contains "$name corrected phase member" $move.Member $correctedGroup
    if ($variant -eq 'B') { Assert-Contains "$name variant requirement" $move.Requires 'WEBSWING_VARIANT_B' }
    else { Assert-Excludes "$name is the fallback variant" $move.Requires 'WEBSWING_VARIANT_B' }
    Assert-Contains "$name resumes after lifecycle phase" $move.Interrupts $phasedGroup
    if ($name -like 'WEBSWING_FULL_ATTACHED*') {
        Assert-Excludes "$name cannot replace a more-specific physical phase" $move.Interrupts $correctedGroup
    }
    else {
        Assert-Contains "$name can replace another physical phase" $move.Interrupts $correctedGroup
    }
    Assert-Contains "$name advances" $move.Flags 'Cycle'
    Assert-Contains "$name holds its terminal pose instead of wrapping" $move.Flags 'HoldLastFrame'
    Assert-True "$name moving window" ($move.Scale -gt 0 -and $move.AnimEnd -gt $move.AnimStart)
    Assert-Excludes "$name excludes MOVEIRQ" $move.Member '<MOVEIRQ>'
}

Assert-True 'A descend uses progressive catch-to-trail motion' ((Get-Move 'WEBSWING_FULL_DESCEND_START').Animation -eq 'MALE/COHSOURCEDEV_WEBSWING_GROUND_LAUNCH_V2')
Assert-True 'B phases use alternate compact performance' (@($phaseMoves | Where-Object { $_.EndsWith('_ALT') -and (Get-Move $_).Animation -ne 'MALE/COHSOURCEDEV_WEBSWING_COMPACT_V2' }).Count -eq 0)
Assert-True 'core outranks phase moves but yields to lifecycle moves' ($core.Priority -gt 27 -and $core.Priority -lt (Get-Move 'WEBSWING_V2_RETRACT_START').Priority -and $core.Priority -lt (Get-Move 'WEBSWING_V2_SHOOT_START').Priority)
Assert-True 'release A uses dedicated unwind motion' ((Get-Move 'WEBSWING_V2_RELEASE_A').Animation -eq 'MALE/COHSOURCEDEV_WEBSWING_RELEASE_V2')

$resolvedPath = (Resolve-Path -LiteralPath $IncludePath).Path
$sha256 = (Get-FileHash -LiteralPath $resolvedPath -Algorithm SHA256).Hash.ToLowerInvariant()
Write-Host 'WEB SWING PHASE CHOREOGRAPHY CHECK PASS'
Write-Host "Include: $resolvedPath"
Write-Host "SHA256: $sha256"
Write-Host "Assertions: $checkCount"
Write-Host 'Lifecycle: reach, catch, release, airborne; physical: descend, bottom, ascend; variants: A/B'
