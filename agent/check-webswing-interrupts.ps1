[CmdletBinding()]
param(
    [string]$IncludePath = (Join-Path $PSScriptRoot 'webswing-animation\webswing.inc')
)

$ErrorActionPreference = 'Stop'

if (-not (Test-Path -LiteralPath $IncludePath -PathType Leaf)) {
    throw "Web Swing sequencer include was not found: $IncludePath"
}

$moves = @{}
$current = $null
foreach ($line in Get-Content -LiteralPath $IncludePath) {
    if ($line -match '^\s*Move\s+(\S+)\s*$') {
        $current = [ordered]@{
            Name = $Matches[1]
            Member = @()
            Interrupts = @()
            Requires = @()
            Flags = @()
            NextMove = $null
            Scale = 1.0
            Animation = $null
            AnimStart = $null
            AnimEnd = $null
        }
        $moves[$current.Name] = $current
        continue
    }
    if ($line -match '^\s*MEnd\s*$') {
        $current = $null
        continue
    }
    if ($null -eq $current) {
        continue
    }

    if ($line -match '^\s*Scale\s+([0-9.]+)\s*$') {
        $current.Scale = [double]::Parse($Matches[1], [Globalization.CultureInfo]::InvariantCulture)
        continue
    }
    if ($line -match '^\s*Anim\s+(\S+)\s+(\d+)\s+(\d+)\s*$') {
        $current.Animation = $Matches[1]
        $current.AnimStart = [int]$Matches[2]
        $current.AnimEnd = [int]$Matches[3]
        continue
    }
    if ($line -match '^\s*NextMove\s+(\S+)\s*$') {
        $current.NextMove = $Matches[1]
        continue
    }
    if ($line -match '^\s*Flags\s+(.+)$') {
        $current.Flags = @($Matches[1] -split ',' | ForEach-Object { $_.Trim() })
        continue
    }
    if ($line -notmatch '^\s*(Member|Interrupts|Requires)\s+(.+)$') {
        continue
    }

    $field = $Matches[1]
    $values = @([regex]::Matches($Matches[2], '"([^"]+)"') | ForEach-Object { $_.Groups[1].Value })
    if ($values.Count -eq 0 -and $field -eq 'Requires') {
        $values = @($Matches[2] -split ',' | ForEach-Object { $_.Trim().Trim('"') } | Where-Object { $_ })
    }
    if ($values.Count -eq 0) {
        throw "Move $($current.Name) has an unreadable $field line: $line"
    }
    $current[$field] = $values
}

function Get-Move([string]$Name) {
    if (-not $moves.ContainsKey($Name)) {
        throw "Required move is missing: $Name"
    }
    return $moves[$Name]
}

# Mirrors Common/seq/seqsequence.c:seqAInterruptsB(candidate, current).
function Test-Interrupt([string]$CandidateName, [string]$CurrentName) {
    $candidate = Get-Move $CandidateName
    $currentMove = Get-Move $CurrentName
    return @($candidate.Interrupts | Where-Object { $currentMove.Member -contains $_ }).Count -gt 0
}

$checkCount = 0
function Assert-Equal([string]$Label, [bool]$Actual, [bool]$Expected) {
    $script:checkCount++
    if ($Actual -ne $Expected) {
        throw "FAIL: $Label (expected=$Expected actual=$Actual)"
    }
}

function Assert-Contains([string]$Label, [string[]]$Values, [string]$Expected) {
    Assert-Equal $Label ($Values -contains $Expected) $true
}

function Assert-Excludes([string]$Label, [string[]]$Values, [string]$Excluded) {
    Assert-Equal $Label ($Values -contains $Excluded) $false
}

$phases = @('ATTACHED', 'DESCEND', 'BOTTOM', 'ASCEND')
$genericGroup = '<WEBSWING_ANIM>'
$correctedGroup = '<WEBSWING_MALE_FULL_CORRECTED>'
$shootGroup = '<WEBSWING_V2_SHOOT>'
$retractGroup = '<WEBSWING_V2_RETRACT>'
$groundLaunchGroup = '<WEBSWING_V2_GROUND_LAUNCH>'
$standardMembers = @('<DEATHIRQ>', '<HITIRQ>', '<REACTIRQ>', '<BLOCKIRQ>', '<BLOCK>', '<STUNMOVE>', '<ATTACKIRQ>')
$standardInterrupts = @('<JUMPS>', '<FALL>', '<GROUNDMOVEALL>')
$phaseWindows = @{
    ATTACHED = [pscustomobject]@{ Start = 1; End = 8; Scale = 0.35 }
    DESCEND = [pscustomobject]@{ Start = 9; End = 17; Scale = 0.22 }
    BOTTOM = [pscustomobject]@{ Start = 18; End = 22; Scale = 0.25 }
    ASCEND = [pscustomobject]@{ Start = 23; End = 40; Scale = 0.32 }
}

foreach ($phase in $phases) {
    $start = Get-Move "WEBSWING_FULL_${phase}_START"
    $hold = Get-Move "WEBSWING_FULL_${phase}_HOLD"
    $window = $phaseWindows[$phase]

    foreach ($move in @($start, $hold)) {
        Assert-Equal "$($move.Name) advances instead of freezing a terminal pose" ($move.Scale -gt 0.0) $true
        Assert-Equal "$($move.Name) uses the accepted $phase frame window" (
            $move.AnimStart -eq $window.Start -and $move.AnimEnd -eq $window.End
        ) $true
        Assert-Equal "$($move.Name) is tempo-matched to the assisted $phase phase" (
            [math]::Abs($move.Scale - $window.Scale) -lt 0.0001
        ) $true
    }

    Assert-Contains "$($start.Name) is a corrected-mode member" $start.Member $correctedGroup
    Assert-Excludes "$($start.Name) is not a generic Web Swing member" $start.Member $genericGroup
    Assert-Contains "$($start.Name) can replace corrected-mode moves" $start.Interrupts $correctedGroup
    Assert-Contains "$($start.Name) can replace generic Web Swing moves" $start.Interrupts $genericGroup
    Assert-Contains "$($start.Name) owns its phase without falling through to stock" $start.Flags 'Cycle'
    Assert-Equal "$($start.Name) has no end-of-window NextMove race" ([string]::IsNullOrEmpty($start.NextMove)) $true
    Assert-Excludes "$($start.Name) remains eligible for the complete controller phase" $start.Requires 'WEBSWING_ASCEND_MALE_ENTER'
    Assert-Excludes "$($start.Name) does not cut off the accepted ground launch" $start.Interrupts $groundLaunchGroup
    foreach ($group in @($retractGroup, $shootGroup)) {
        Assert-Contains "$($start.Name) can resume directly after choreography $group" $start.Interrupts $group
    }

    Assert-Contains "$($hold.Name) is a corrected-mode member" $hold.Member $correctedGroup
    Assert-Excludes "$($hold.Name) is not a generic Web Swing member" $hold.Member $genericGroup
    Assert-Excludes "$($hold.Name) cannot replace corrected-mode moves" $hold.Interrupts $correctedGroup
    Assert-Contains "$($hold.Name) can replace generic Web Swing moves" $hold.Interrupts $genericGroup

    foreach ($group in $standardMembers) {
        Assert-Contains "$($start.Name) retains stock membership $group" $start.Member $group
        Assert-Contains "$($hold.Name) retains stock membership $group" $hold.Member $group
    }
    Assert-Excludes "$($start.Name) excludes MOVEIRQ to block FFLY_PREFALL interruption" $start.Member '<MOVEIRQ>'
    Assert-Excludes "$($hold.Name) excludes MOVEIRQ to block FFLY_PREFALL interruption" $hold.Member '<MOVEIRQ>'
    foreach ($group in $standardInterrupts) {
        Assert-Contains "$($start.Name) retains stock interrupt $group" $start.Interrupts $group
        Assert-Contains "$($hold.Name) retains stock interrupt $group" $hold.Interrupts $group
    }

    Assert-Equal "$($hold.Name) does not interrupt $($start.Name)" (Test-Interrupt $hold.Name $start.Name) $false

    $generic = "WEBSWING_$phase"
    foreach ($corrected in @($start.Name, $hold.Name)) {
        Assert-Equal "$generic does not interrupt $corrected" (Test-Interrupt $generic $corrected) $false
    }
}

foreach ($newPhase in $phases) {
    $candidate = "WEBSWING_FULL_${newPhase}_START"
    foreach ($oldPhase in $phases | Where-Object { $_ -ne $newPhase }) {
        foreach ($suffix in @('START', 'HOLD')) {
            $currentName = "WEBSWING_FULL_${oldPhase}_$suffix"
            Assert-Equal "$candidate interrupts $currentName" (Test-Interrupt $candidate $currentName) $true
        }
    }
}

$choreography = @(
    [pscustomobject]@{ Label = 'ground launch'; Prefix = 'WEBSWING_V2_GROUND_LAUNCH'; Group = $groundLaunchGroup },
    [pscustomobject]@{ Label = 'retract'; Prefix = 'WEBSWING_V2_RETRACT'; Group = $retractGroup },
    [pscustomobject]@{ Label = 'shoot'; Prefix = 'WEBSWING_V2_SHOOT'; Group = $shootGroup }
)
foreach ($item in $choreography) {
    $start = Get-Move "$($item.Prefix)_START"
    $hold = Get-Move "$($item.Prefix)_HOLD"
    Assert-Equal "$($hold.Name) advances instead of freezing a terminal pose" ($hold.Scale -gt 0.0) $true
    Assert-Equal "$($hold.Name) retains more than one authored frame" ($hold.AnimEnd -gt $hold.AnimStart) $true
    foreach ($move in @($start, $hold)) {
        Assert-Contains "$($move.Name) is a V2 $($item.Label) member" $move.Member $item.Group
        Assert-Excludes "$($move.Name) is isolated from corrected phase membership" $move.Member $correctedGroup
        Assert-Excludes "$($move.Name) excludes MOVEIRQ to block FFLY_PREFALL interruption" $move.Member '<MOVEIRQ>'
        foreach ($group in $standardMembers) {
            Assert-Contains "$($move.Name) retains stock membership $group" $move.Member $group
        }
        foreach ($group in $standardInterrupts) {
            Assert-Contains "$($move.Name) retains stock interrupt $group" $move.Interrupts $group
        }
    }
    Assert-Contains "$($item.Label) START can replace corrected phase moves" $start.Interrupts $correctedGroup
    Assert-Contains "$($item.Label) START can replace stale $($item.Label) choreography" $start.Interrupts $item.Group
    Assert-Contains "$($item.Label) START can replace generic Web Swing moves" $start.Interrupts $genericGroup
    Assert-Excludes "$($item.Label) HOLD cannot replace corrected phase moves" $hold.Interrupts $correctedGroup
    Assert-Excludes "$($item.Label) HOLD cannot restart its choreography" $hold.Interrupts $item.Group
    Assert-Equal "$($item.Label) HOLD does not interrupt its START" (Test-Interrupt $hold.Name $start.Name) $false
}

$launchStart = Get-Move 'WEBSWING_V2_GROUND_LAUNCH_START'
$launchHold = Get-Move 'WEBSWING_V2_GROUND_LAUNCH_HOLD'
$retractStart = Get-Move 'WEBSWING_V2_RETRACT_START'
$retractHold = Get-Move 'WEBSWING_V2_RETRACT_HOLD'
$shootStart = Get-Move 'WEBSWING_V2_SHOOT_START'
$shootHold = Get-Move 'WEBSWING_V2_SHOOT_HOLD'

Assert-Equal 'retract START reuses the complete accepted launch-to-tuck performance' (
    $retractStart.Animation -eq 'MALE/COHSOURCEDEV_WEBSWING_GROUND_LAUNCH_V2' -and
    $retractStart.AnimStart -eq 1 -and $retractStart.AnimEnd -eq 62
) $true
Assert-Equal 'retract START hands a long ascent to its moving tuck tail' (
    $retractStart.NextMove -eq 'WEBSWING_V2_RETRACT_HOLD'
) $true
Assert-Contains 'retract START selects once per new assisted segment' $retractStart.Requires 'WEBSWING_ASCEND_MALE_ENTER'
Assert-Excludes 'retract START does not wrap the full performance' $retractStart.Flags 'Cycle'
Assert-Equal 'retract START spans the complete assisted arc instead of dwelling in tuck' $retractStart.Scale 0.5
Assert-Equal 'retract HOLD reuses only the moving tuck tail' (
    $retractHold.Animation -eq 'MALE/COHSOURCEDEV_WEBSWING_GROUND_LAUNCH_V2' -and
    $retractHold.AnimStart -eq 54 -and $retractHold.AnimEnd -eq 62
) $true
Assert-Contains 'retract HOLD keeps its tuck tail alive' $retractHold.Flags 'Cycle'

Assert-Contains 'fresh airborne shoot cycles its complete authored clip while active' $shootStart.Flags 'Cycle'
Assert-Equal 'fresh airborne shoot has no terminal HOLD fallthrough' ([string]::IsNullOrEmpty($shootStart.NextMove)) $true
Assert-Excludes 'fresh airborne shoot does not depend on a one-tick entry pulse' $shootStart.Requires 'WEBSWING_ASCEND_MALE_ENTER'
Assert-Equal 'ground launch still completes its authored one-shot before its hold' (
    $launchStart.NextMove -eq 'WEBSWING_V2_GROUND_LAUNCH_HOLD'
) $true
Assert-Equal 'ground launch uses the complete accepted launch-to-tuck performance' (
    $launchStart.Animation -eq 'MALE/COHSOURCEDEV_WEBSWING_GROUND_LAUNCH_V2' -and
    $launchStart.AnimStart -eq 1 -and $launchStart.AnimEnd -eq 62
) $true

foreach ($old in @($retractStart.Name, $retractHold.Name, $shootStart.Name, $shootHold.Name)) {
    Assert-Equal "ground launch START interrupts $old" (Test-Interrupt $launchStart.Name $old) $true
}
foreach ($old in @($shootStart.Name, $shootHold.Name)) {
    Assert-Equal "retract START interrupts $old" (Test-Interrupt $retractStart.Name $old) $true
}
foreach ($old in @($launchStart.Name, $launchHold.Name)) {
    Assert-Equal "new-cycle retract START can replace $old" (Test-Interrupt $retractStart.Name $old) $true
}
foreach ($old in @($retractStart.Name, $retractHold.Name)) {
    Assert-Equal "shoot START interrupts $old" (Test-Interrupt $shootStart.Name $old) $true
}
foreach ($old in @($launchStart.Name, $launchHold.Name)) {
    Assert-Equal "shoot START cannot cut off $old" (Test-Interrupt $shootStart.Name $old) $false
}

foreach ($phase in $phases) {
    $phaseStart = "WEBSWING_FULL_${phase}_START"
    $phaseHold = "WEBSWING_FULL_${phase}_HOLD"
    foreach ($item in $choreography) {
        $start = "$($item.Prefix)_START"
        $hold = "$($item.Prefix)_HOLD"
        $phaseCanResume = $item.Prefix -ne 'WEBSWING_V2_GROUND_LAUNCH'
        Assert-Equal "$phaseStart respects $start completion policy" (Test-Interrupt $phaseStart $start) $phaseCanResume
        Assert-Equal "$phaseStart respects $hold completion policy" (Test-Interrupt $phaseStart $hold) $phaseCanResume
        Assert-Equal "$start interrupts $phaseStart" (Test-Interrupt $start $phaseStart) $true
        Assert-Equal "$start interrupts $phaseHold" (Test-Interrupt $start $phaseHold) $true
    }
}

$resolvedPath = (Resolve-Path -LiteralPath $IncludePath).Path
$sha256 = (Get-FileHash -LiteralPath $resolvedPath -Algorithm SHA256).Hash.ToLowerInvariant()
Write-Host 'WEB SWING INTERRUPT CHECK PASS'
Write-Host "Include: $resolvedPath"
Write-Host "SHA256: $sha256"
Write-Host "Assertions: $checkCount"
Write-Host 'Same-phase HOLD -> START: 4/4 blocked'
Write-Host 'Generic fallback -> corrected START/HOLD: 8/8 blocked'
Write-Host 'Cross-phase corrected START -> prior START/HOLD: 24/24 allowed'
Write-Host 'Mode-3 phase cycles and direct choreography recovery: verified'
