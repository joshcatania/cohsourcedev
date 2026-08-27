[CmdletBinding()]
param(
    # Omit this to package the newest explicit client capture.
    [string]$InputPath = '',
    [string]$OutputRoot = '',
    # Optional future-reference artifact.  No optimization or retuning is done.
    [int]$GoldenCycle = 0,
    [switch]$Json
)

$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path -Parent $PSScriptRoot
$invariant = [Globalization.CultureInfo]::InvariantCulture
$physicsTickRate = 30.0
$sampleRate = 15.0

function Get-TextValue {
    param([object]$Row, [string]$Name)
    if ($null -eq $Row) { return '' }
    $property = $Row.PSObject.Properties[$Name]
    if ($null -eq $property -or $null -eq $property.Value) { return '' }
    return [string]$property.Value
}

function Get-NumberValue {
    param([object]$Row, [string]$Name)
    $raw = Get-TextValue -Row $Row -Name $Name
    if ([string]::IsNullOrWhiteSpace($raw)) { return $null }
    $number = 0.0
    if ([double]::TryParse($raw, [Globalization.NumberStyles]::Float, $invariant, [ref]$number)) {
        return $number
    }
    return $null
}

function Get-Mean {
    param([object[]]$Values)
    $items = @($Values | Where-Object { $null -ne $_ })
    if ($items.Count -eq 0) { return $null }
    $sum = 0.0
    foreach ($item in $items) { $sum += [double]$item }
    return $sum / $items.Count
}

function Get-Minimum {
    param([object[]]$Values)
    $items = @($Values | Where-Object { $null -ne $_ })
    if ($items.Count -eq 0) { return $null }
    $minimum = [double]$items[0]
    foreach ($item in $items) {
        if ([double]$item -lt $minimum) { $minimum = [double]$item }
    }
    return $minimum
}

function Get-Maximum {
    param([object[]]$Values)
    $items = @($Values | Where-Object { $null -ne $_ })
    if ($items.Count -eq 0) { return $null }
    $maximum = [double]$items[0]
    foreach ($item in $items) {
        if ([double]$item -gt $maximum) { $maximum = [double]$item }
    }
    return $maximum
}

function Get-StdDev {
    param([object[]]$Values)
    $items = @($Values | Where-Object { $null -ne $_ })
    if ($items.Count -lt 2) { return $null }
    $mean = Get-Mean -Values $items
    $sum = 0.0
    foreach ($item in $items) {
        $delta = [double]$item - $mean
        $sum += $delta * $delta
    }
    # Population standard deviation describes the cycles in this capture.
    return [Math]::Sqrt($sum / $items.Count)
}

function Get-RowClearance {
    param([object]$Row)
    $values = @()
    if ($null -ne $Row.CurrentClearance -and [double]$Row.CurrentClearance -ge 0.0) {
        $values += [double]$Row.CurrentClearance
    }
    if ($null -ne $Row.AheadClearance -and [double]$Row.AheadClearance -ge 0.0) {
        $values += [double]$Row.AheadClearance
    }
    return Get-Minimum -Values $values
}

function Format-Value {
    param([object]$Value, [int]$Digits = 3)
    if ($null -eq $Value) { return 'n/a' }
    $format = 'F{0}' -f $Digits
    return ([double]$Value).ToString($format, $invariant)
}

function Format-Percent {
    param([object]$Value)
    if ($null -eq $Value) { return 'n/a' }
    return ([double]$Value).ToString('F1', $invariant) + '%'
}

function Escape-SvgText {
    param([string]$Value)
    return [System.Security.SecurityElement]::Escape($Value)
}

function Write-LineSvg {
    param(
        [string]$Path,
        [object[]]$Rows,
        [string]$Property,
        [string]$Title,
        [string]$Color
    )

    $width = 1000.0
    $height = 360.0
    $left = 70.0
    $right = 20.0
    $top = 38.0
    $bottom = 48.0
    $plotWidth = $width - $left - $right
    $plotHeight = $height - $top - $bottom
    $data = @($Rows | Where-Object {
        $tick = $_.Tick
        $value = $_.$Property
        $validValue = $null -ne $value
        if ($validValue -and ($Property -eq 'CurrentClearance' -or $Property -eq 'AheadClearance' -or $Property -eq 'LookaheadDistance')) {
            $validValue = [double]$value -ge 0.0
        }
        $null -ne $tick -and $validValue
    })

    $sb = New-Object System.Text.StringBuilder
    [void]$sb.AppendLine('<svg xmlns="http://www.w3.org/2000/svg" width="1000" height="360" viewBox="0 0 1000 360">')
    [void]$sb.AppendLine('<rect width="100%" height="100%" fill="#10151b"/>')
    [void]$sb.AppendLine(('<text x="70" y="24" fill="#f0f4f8" font-family="sans-serif" font-size="16">{0}</text>' -f (Escape-SvgText $Title)))
    [void]$sb.AppendLine(('<line x1="{0}" y1="{1}" x2="{2}" y2="{1}" stroke="#65717d"/><line x1="{0}" y1="{1}" x2="{0}" y2="{3}" stroke="#65717d"/>' -f $left, ($height - $bottom), ($width - $right), $top))

    if ($data.Count -eq 0) {
        [void]$sb.AppendLine(('<text x="70" y="190" fill="#c6d0d9" font-family="sans-serif" font-size="14">No valid {0} samples</text>' -f (Escape-SvgText $Property)))
    } else {
        $ticks = @($data | ForEach-Object { [double]$_.Tick })
        $values = @($data | ForEach-Object { [double]$_.$Property })
        $minTick = Get-Minimum -Values $ticks
        $maxTick = Get-Maximum -Values $ticks
        $minValue = Get-Minimum -Values $values
        $maxValue = Get-Maximum -Values $values
        if ($maxTick -eq $minTick) { $maxTick = $minTick + 1.0 }
        if ($maxValue -eq $minValue) {
            $minValue -= 1.0
            $maxValue += 1.0
        }

        [void]$sb.AppendLine(('<text x="8" y="{0}" fill="#c6d0d9" font-family="sans-serif" font-size="12">{1}</text>' -f ($top + 8), (Format-Value $maxValue 2)))
        [void]$sb.AppendLine(('<text x="8" y="{0}" fill="#c6d0d9" font-family="sans-serif" font-size="12">{1}</text>' -f ($height - $bottom), (Format-Value $minValue 2)))
        [void]$sb.AppendLine(('<text x="{0}" y="{1}" fill="#c6d0d9" font-family="sans-serif" font-size="12">tick {2}</text>' -f $left, ($height - 12), ([int64]$minTick)))
        [void]$sb.AppendLine(('<text x="{0}" y="{1}" fill="#c6d0d9" font-family="sans-serif" font-size="12">tick {2}</text>' -f ($width - $right - 85), ($height - 12), ([int64]$maxTick)))

        $points = New-Object System.Text.StringBuilder
        foreach ($item in $data) {
            $x = $left + (([double]$item.Tick - $minTick) / ($maxTick - $minTick)) * $plotWidth
            $y = ($height - $bottom) - (([double]$item.$Property - $minValue) / ($maxValue - $minValue)) * $plotHeight
            [void]$points.Append((Format-Value $x 2))
            [void]$points.Append(',')
            [void]$points.Append((Format-Value $y 2))
            [void]$points.Append(' ')
        }
        [void]$sb.AppendLine(('<polyline fill="none" stroke="{0}" stroke-width="2" points="{1}"/>' -f $Color, $points.ToString().Trim()))
    }

    [void]$sb.AppendLine('</svg>')
    [IO.File]::WriteAllText($Path, $sb.ToString(), (New-Object System.Text.UTF8Encoding($false)))
}

function Write-TopDownSvg {
    param([string]$Path, [object[]]$Rows)

    $width = 700.0
    $height = 520.0
    $left = 55.0
    $right = 25.0
    $top = 38.0
    $bottom = 38.0
    $data = @($Rows | Where-Object { $null -ne $_.PosX -and $null -ne $_.PosZ })
    $sb = New-Object System.Text.StringBuilder
    [void]$sb.AppendLine('<svg xmlns="http://www.w3.org/2000/svg" width="700" height="520" viewBox="0 0 700 520">')
    [void]$sb.AppendLine('<rect width="100%" height="100%" fill="#10151b"/>')
    [void]$sb.AppendLine('<text x="55" y="24" fill="#f0f4f8" font-family="sans-serif" font-size="16">Top-down path (X/Z)</text>')
    [void]$sb.AppendLine(('<line x1="{0}" y1="{1}" x2="{2}" y2="{1}" stroke="#65717d"/><line x1="{0}" y1="{1}" x2="{0}" y2="{3}" stroke="#65717d"/>' -f $left, ($height - $bottom), ($width - $right), $top))

    if ($data.Count -eq 0) {
        [void]$sb.AppendLine('<text x="55" y="270" fill="#c6d0d9" font-family="sans-serif" font-size="14">No valid position samples</text>')
    } else {
        $xs = @($data | ForEach-Object { [double]$_.PosX })
        $zs = @($data | ForEach-Object { [double]$_.PosZ })
        $minX = Get-Minimum -Values $xs
        $maxX = Get-Maximum -Values $xs
        $minZ = Get-Minimum -Values $zs
        $maxZ = Get-Maximum -Values $zs
        if ($maxX -eq $minX) { $minX -= 1.0; $maxX += 1.0 }
        if ($maxZ -eq $minZ) { $minZ -= 1.0; $maxZ += 1.0 }
        $plotWidth = $width - $left - $right
        $plotHeight = $height - $top - $bottom
        $points = New-Object System.Text.StringBuilder
        foreach ($item in $data) {
            $x = $left + (([double]$item.PosX - $minX) / ($maxX - $minX)) * $plotWidth
            $y = ($height - $bottom) - (([double]$item.PosZ - $minZ) / ($maxZ - $minZ)) * $plotHeight
            [void]$points.Append((Format-Value $x 2))
            [void]$points.Append(',')
            [void]$points.Append((Format-Value $y 2))
            [void]$points.Append(' ')
        }
        [void]$sb.AppendLine(('<polyline fill="none" stroke="#55c7ff" stroke-width="2" points="{0}"/>' -f $points.ToString().Trim()))
        $first = $data[0]
        $last = $data[$data.Count - 1]
        $firstX = $left + (([double]$first.PosX - $minX) / ($maxX - $minX)) * $plotWidth
        $firstY = ($height - $bottom) - (([double]$first.PosZ - $minZ) / ($maxZ - $minZ)) * $plotHeight
        $lastX = $left + (([double]$last.PosX - $minX) / ($maxX - $minX)) * $plotWidth
        $lastY = ($height - $bottom) - (([double]$last.PosZ - $minZ) / ($maxZ - $minZ)) * $plotHeight
        [void]$sb.AppendLine(('<circle cx="{0}" cy="{1}" r="5" fill="#8be28b"/><circle cx="{2}" cy="{3}" r="5" fill="#ffb454"/>' -f (Format-Value $firstX 2), (Format-Value $firstY 2), (Format-Value $lastX 2), (Format-Value $lastY 2)))
        [void]$sb.AppendLine(('<text x="55" y="{0}" fill="#c6d0d9" font-family="sans-serif" font-size="12">X {1} .. {2}</text>' -f ($height - 12), (Format-Value $minX 1), (Format-Value $maxX 1)))
        [void]$sb.AppendLine(('<text x="520" y="{0}" fill="#8be28b" font-family="sans-serif" font-size="12">start</text><text x="575" y="{0}" fill="#ffb454" font-family="sans-serif" font-size="12">end</text>' -f ($height - 12)))
    }

    [void]$sb.AppendLine('</svg>')
    [IO.File]::WriteAllText($Path, $sb.ToString(), (New-Object System.Text.UTF8Encoding($false)))
}

function Get-LatestCaptureDirectory {
    param([string]$Root)
    if (-not (Test-Path -LiteralPath $Root)) {
        throw "No raw Web Swing capture root exists: $Root"
    }
    $latest = Get-ChildItem -LiteralPath $Root -Filter 'telemetry.csv' -File -Recurse |
        Sort-Object LastWriteTime -Descending | Select-Object -First 1
    if ($null -eq $latest) {
        throw "No telemetry.csv found under $Root. Start a capture with /webswingcapture 1."
    }
    return $latest.Directory.FullName
}

function Assert-Headers {
    param([string]$Path, [string[]]$Required)
    $headerLine = Get-Content -LiteralPath $Path -TotalCount 1
    if ([string]::IsNullOrWhiteSpace($headerLine)) { throw "CSV is empty: $Path" }
    $headers = @($headerLine.Split(',') | ForEach-Object { $_.Trim() })
    foreach ($name in $Required) {
        if ($headers -notcontains $name) { throw "CSV $Path is missing required column '$name'" }
    }
}

function Get-CsvHeaders {
    param([string]$Path)
    $headerLine = Get-Content -LiteralPath $Path -TotalCount 1
    if ([string]::IsNullOrWhiteSpace($headerLine)) { throw "CSV is empty: $Path" }
    return @($headerLine.Split(',') | ForEach-Object { $_.Trim() })
}

try {
    if ([string]::IsNullOrWhiteSpace($InputPath)) {
        $inputDirectory = Get-LatestCaptureDirectory -Root (Join-Path $repoRoot 'bin\logs\game\webswing-captures')
    } else {
        $resolvedInput = Resolve-Path -LiteralPath $InputPath
        $inputItem = Get-Item -LiteralPath $resolvedInput.Path
        $inputDirectory = if ($inputItem.PSIsContainer) { $inputItem.FullName } else { $inputItem.Directory.FullName }
    }

    $telemetryPath = Join-Path $inputDirectory 'telemetry.csv'
    $eventsPath = Join-Path $inputDirectory 'events.csv'
    if (-not (Test-Path -LiteralPath $telemetryPath)) { throw "Capture is missing telemetry.csv: $inputDirectory" }
    if (-not (Test-Path -LiteralPath $eventsPath)) { throw "Capture is missing events.csv: $inputDirectory" }
    Assert-Headers -Path $telemetryPath -Required @(
        'capture_id', 'tick', 'sample_index', 'backend', 'phase', 'phase_ticks', 'cycle_id',
        'pos_x', 'pos_y', 'pos_z', 'vel_x', 'vel_y', 'vel_z', 'total_speed',
        'horizontal_speed', 'vertical_speed'
    )
    Assert-Headers -Path $eventsPath -Required @('event_type', 'capture_id', 'tick', 'cycle_id', 'phase')

    $telemetryHeaders = Get-CsvHeaders -Path $telemetryPath
    $eventHeaders = Get-CsvHeaders -Path $eventsPath
    $identityHeaders = @('activation_id', 'capture_cycle_index', 'controller_cycle_id')
    $telemetryIdentityCount = @($identityHeaders | Where-Object { $telemetryHeaders -contains $_ }).Count
    $eventIdentityCount = @($identityHeaders | Where-Object { $eventHeaders -contains $_ }).Count
    if ($telemetryIdentityCount -ne 0 -and $telemetryIdentityCount -ne $identityHeaders.Count) {
        throw "Telemetry has a partial capture identity schema; expected all of $($identityHeaders -join ', ')."
    }
    if ($eventIdentityCount -ne 0 -and $eventIdentityCount -ne $identityHeaders.Count) {
        throw "Events have a partial capture identity schema; expected all of $($identityHeaders -join ', ')."
    }
    if ($telemetryIdentityCount -eq $identityHeaders.Count -and $eventIdentityCount -ne $identityHeaders.Count) {
        throw 'Telemetry has capture-local identity fields but events.csv does not; refusing an ambiguous package.'
    }
    $captureIdentityAvailable = $telemetryIdentityCount -eq $identityHeaders.Count

    $rawRows = @(Import-Csv -LiteralPath $telemetryPath)
    $eventRows = @(Import-Csv -LiteralPath $eventsPath)
    if ($rawRows.Count -eq 0) { throw "Capture telemetry has no samples: $telemetryPath" }

    $normalizedList = New-Object System.Collections.Generic.List[object]
    foreach ($raw in $rawRows) {
        $tickValue = Get-NumberValue -Row $raw -Name 'tick'
        if ($null -eq $tickValue) { continue }
        $sampleValue = Get-NumberValue -Row $raw -Name 'sample_index'
        $cycleValue = Get-NumberValue -Row $raw -Name 'cycle_id'
        $activationValue = Get-NumberValue -Row $raw -Name 'activation_id'
        $captureCycleValue = Get-NumberValue -Row $raw -Name 'capture_cycle_index'
        $controllerCycleValue = Get-NumberValue -Row $raw -Name 'controller_cycle_id'
        if ($null -eq $controllerCycleValue) { $controllerCycleValue = $cycleValue }
        $phase = Get-TextValue -Row $raw -Name 'phase'
        if ([string]::IsNullOrWhiteSpace($phase)) { $phase = Get-TextValue -Row $raw -Name 'assist_phase' }
        $normalizedList.Add([pscustomobject][ordered]@{
            CaptureId = Get-NumberValue -Row $raw -Name 'capture_id'
            Tick = [int64]$tickValue
            SampleIndex = if ($null -eq $sampleValue) { 0 } else { [int64]$sampleValue }
            Backend = Get-TextValue -Row $raw -Name 'backend'
            SwingEnabled = Get-NumberValue -Row $raw -Name 'swing_enabled'
            Attached = Get-NumberValue -Row $raw -Name 'attached'
            AssistPhase = Get-TextValue -Row $raw -Name 'assist_phase'
            Phase = $phase
            PhaseTicks = if ($null -eq (Get-NumberValue -Row $raw -Name 'phase_ticks')) { 0 } else { [int64](Get-NumberValue -Row $raw -Name 'phase_ticks') }
            ActivationId = if ($null -eq $activationValue) { 0 } else { [int64]$activationValue }
            CaptureCycleIndex = if ($null -eq $captureCycleValue) {
                if ($null -eq $controllerCycleValue) { 0 } else { [int64]$controllerCycleValue }
            } else { [int64]$captureCycleValue }
            ControllerCycleId = if ($null -eq $controllerCycleValue) { 0 } else { [int64]$controllerCycleValue }
            # Keep the existing normalized name as the controller diagnostic
            # value for callers of the original packaging script.
            CycleId = if ($null -eq $controllerCycleValue) { 0 } else { [int64]$controllerCycleValue }
            AssistEnergy = Get-NumberValue -Row $raw -Name 'assist_energy'
            PosX = Get-NumberValue -Row $raw -Name 'pos_x'
            PosY = Get-NumberValue -Row $raw -Name 'pos_y'
            PosZ = Get-NumberValue -Row $raw -Name 'pos_z'
            VelX = Get-NumberValue -Row $raw -Name 'vel_x'
            VelY = Get-NumberValue -Row $raw -Name 'vel_y'
            VelZ = Get-NumberValue -Row $raw -Name 'vel_z'
            TotalSpeed = Get-NumberValue -Row $raw -Name 'total_speed'
            HorizontalSpeed = Get-NumberValue -Row $raw -Name 'horizontal_speed'
            VerticalSpeed = Get-NumberValue -Row $raw -Name 'vertical_speed'
            CurrentClearance = Get-NumberValue -Row $raw -Name 'current_ground_clearance'
            AheadClearance = Get-NumberValue -Row $raw -Name 'ahead_ground_clearance'
            LookaheadDistance = Get-NumberValue -Row $raw -Name 'lookahead_distance'
            LowPointY = Get-NumberValue -Row $raw -Name 'low_point_y'
            InitialLowPointY = Get-NumberValue -Row $raw -Name 'initial_low_point_y'
            AltitudeMargin = Get-NumberValue -Row $raw -Name 'altitude_margin'
            IntentX = Get-NumberValue -Row $raw -Name 'intent_x'
            IntentY = Get-NumberValue -Row $raw -Name 'intent_y'
            IntentZ = Get-NumberValue -Row $raw -Name 'intent_z'
            InputMagnitude = Get-NumberValue -Row $raw -Name 'input_magnitude'
            AnchorX = Get-NumberValue -Row $raw -Name 'anchor_x'
            AnchorY = Get-NumberValue -Row $raw -Name 'anchor_y'
            AnchorZ = Get-NumberValue -Row $raw -Name 'anchor_z'
            VisualTetherState = Get-TextValue -Row $raw -Name 'visual_tether_state'
            VisualTetherProgress = Get-NumberValue -Row $raw -Name 'visual_tether_progress'
            AnimPhase = Get-TextValue -Row $raw -Name 'anim_phase'
            AnimSegmentId = if ($null -eq (Get-NumberValue -Row $raw -Name 'anim_segment_id')) { 0 } else { [int64](Get-NumberValue -Row $raw -Name 'anim_segment_id') }
            AnimPhaseSegmentId = if ($null -eq (Get-NumberValue -Row $raw -Name 'anim_phase_segment_id')) { 0 } else { [int64](Get-NumberValue -Row $raw -Name 'anim_phase_segment_id') }
            SwingPlaneSpeed = Get-NumberValue -Row $raw -Name 'swing_plane_speed'
            SwingAngleDeg = Get-NumberValue -Row $raw -Name 'swing_angle_deg'
            ForwardSpeedAlongIntent = Get-NumberValue -Row $raw -Name 'forward_speed_along_intent'
            SwoopActive = Get-NumberValue -Row $raw -Name 'swoop_active'
            SwoopRadius = Get-NumberValue -Row $raw -Name 'swoop_radius'
            SwoopEntryPlaneSpeed = Get-NumberValue -Row $raw -Name 'swoop_entry_plane_speed'
            SwoopEntryAngleDeg = Get-NumberValue -Row $raw -Name 'swoop_entry_angle_deg'
            SwoopEmergencyCount = Get-NumberValue -Row $raw -Name 'swoop_emergency_count'
        })
    }
    $rows = @($normalizedList | Sort-Object Tick, SampleIndex)
    if ($rows.Count -eq 0) { throw "Capture telemetry has no parseable tick rows: $telemetryPath" }

    if ([string]::IsNullOrWhiteSpace($OutputRoot)) {
        $OutputRoot = Join-Path $PSScriptRoot 'captures'
    } elseif (-not [IO.Path]::IsPathRooted($OutputRoot)) {
        $OutputRoot = Join-Path (Get-Location).Path $OutputRoot
    }
    New-Item -ItemType Directory -Force -Path $OutputRoot | Out-Null
    $captureName = Split-Path $inputDirectory -Leaf
    if ([string]::IsNullOrWhiteSpace($captureName)) { $captureName = 'webswing-capture' }
    $outputDirectory = Join-Path $OutputRoot $captureName
    New-Item -ItemType Directory -Force -Path $outputDirectory | Out-Null
    Copy-Item -LiteralPath $telemetryPath -Destination (Join-Path $outputDirectory 'telemetry.csv') -Force
    Copy-Item -LiteralPath $eventsPath -Destination (Join-Path $outputDirectory 'events.csv') -Force
    $metadataPath = Join-Path $inputDirectory 'metadata.json'
    if (Test-Path -LiteralPath $metadataPath) {
        Copy-Item -LiteralPath $metadataPath -Destination (Join-Path $outputDirectory 'metadata.json') -Force
    }

    $metadata = $null
    if (Test-Path -LiteralPath $metadataPath) {
        try { $metadata = Get-Content -LiteralPath $metadataPath -Raw | ConvertFrom-Json } catch { $metadata = $null }
    }

    $first = $rows[0]
    $last = $rows[$rows.Count - 1]
    $durationSeconds = ([double]($last.Tick - $first.Tick)) / $physicsTickRate
    $deltaX = if ($null -ne $first.PosX -and $null -ne $last.PosX) { [double]$last.PosX - $first.PosX } else { 0.0 }
    $deltaY = if ($null -ne $first.PosY -and $null -ne $last.PosY) { [double]$last.PosY - $first.PosY } else { 0.0 }
    $deltaZ = if ($null -ne $first.PosZ -and $null -ne $last.PosZ) { [double]$last.PosZ - $first.PosZ } else { 0.0 }
    $totalDisplacement = [Math]::Sqrt($deltaX * $deltaX + $deltaY * $deltaY + $deltaZ * $deltaZ)
    $horizontalDisplacement = [Math]::Sqrt($deltaX * $deltaX + $deltaZ * $deltaZ)

    $captureCycleIndices = @($rows |
        Where-Object { $_.CaptureCycleIndex -gt 0 } |
        Select-Object -ExpandProperty CaptureCycleIndex -Unique |
        Sort-Object)
    $cycles = @()
    foreach ($captureCycleIndex in $captureCycleIndices) {
        $cycleRows = @($rows | Where-Object { $_.CaptureCycleIndex -eq $captureCycleIndex })
        if ($cycleRows.Count -eq 0) { continue }
        $cycleFirst = $cycleRows[0]
        $cycleLast = $cycleRows[$cycleRows.Count - 1]
        $activationId = $cycleFirst.ActivationId
        $controllerCycleIds = @($cycleRows | Select-Object -ExpandProperty ControllerCycleId -Unique)
        $controllerCycleId = if ($controllerCycleIds.Count -eq 0) { 0 } else { [int64]$controllerCycleIds[0] }
        $apexY = Get-Maximum -Values @($cycleRows | ForEach-Object { $_.PosY })
        $lowY = Get-Minimum -Values @($cycleRows | ForEach-Object { $_.PosY })
        $apexRow = $cycleRows | Sort-Object PosY -Descending | Select-Object -First 1
        $bottomRows = @($cycleRows | Where-Object { $_.Phase -eq 'BOTTOM' -or $_.AssistPhase -eq 'BOTTOM' })
        $bottomSpeedValues = @($bottomRows | ForEach-Object { $_.HorizontalSpeed })
        $bottomSpeed = Get-Mean -Values $bottomSpeedValues
        if ($null -eq $bottomSpeed) {
            $lowestRow = $cycleRows | Sort-Object PosY | Select-Object -First 1
            $bottomSpeed = $lowestRow.HorizontalSpeed
        }
        $apexSpeed = $apexRow.HorizontalSpeed
        $ratio = if ($null -ne $apexSpeed -and [Math]::Abs([double]$apexSpeed) -gt 0.000001 -and $null -ne $bottomSpeed) { [double]$bottomSpeed / [double]$apexSpeed } else { $null }
        $clearanceValues = @($cycleRows | ForEach-Object { Get-RowClearance $_ })
        $minClearance = Get-Minimum -Values $clearanceValues
        $cycleDeltaX = if ($null -ne $cycleFirst.PosX -and $null -ne $cycleLast.PosX) { [double]$cycleLast.PosX - $cycleFirst.PosX } else { 0.0 }
        $cycleDeltaZ = if ($null -ne $cycleFirst.PosZ -and $null -ne $cycleLast.PosZ) { [double]$cycleLast.PosZ - $cycleFirst.PosZ } else { 0.0 }
        $intentLength = if ($null -ne $cycleFirst.IntentX -and $null -ne $cycleFirst.IntentZ) { [Math]::Sqrt([double]$cycleFirst.IntentX * $cycleFirst.IntentX + [double]$cycleFirst.IntentZ * $cycleFirst.IntentZ) } else { 0.0 }
        if ($intentLength -gt 0.001) {
            $forwardDisplacement = ($cycleDeltaX * [double]$cycleFirst.IntentX + $cycleDeltaZ * [double]$cycleFirst.IntentZ) / $intentLength
        } else {
            $forwardDisplacement = [Math]::Sqrt($cycleDeltaX * $cycleDeltaX + $cycleDeltaZ * $cycleDeltaZ)
        }

        $bottomEntry = $cycleRows | Where-Object { $_.Phase -eq 'BOTTOM' -or $_.AssistPhase -eq 'BOTTOM' } | Select-Object -First 1
        $descendingRows = @($cycleRows | Where-Object { $_.VerticalSpeed -lt 0.0 })
        $reversalStart = $bottomEntry
        $reversalDefinition = 'bottom_entry'
        if ($null -eq $reversalStart -and $descendingRows.Count -gt 0) {
            $reversalStart = $descendingRows | Sort-Object VerticalSpeed | Select-Object -First 1
            $reversalDefinition = 'strongest_descending_sample'
        }
        $reversalTicks = $null
        $reversalStatus = 'not_applicable'
        if ($null -ne $reversalStart) {
            $upward = $cycleRows | Where-Object {
                $_.Tick -ge $reversalStart.Tick -and ($_.VerticalSpeed -gt 0.0 -or $_.Phase -eq 'ASCEND' -or $_.AssistPhase -eq 'ASCEND')
            } | Select-Object -First 1
            if ($null -ne $upward) {
                $reversalTicks = [int64]$upward.Tick - [int64]$reversalStart.Tick
                $reversalStatus = 'observed'
            } else {
                $reversalStatus = 'not_observed'
            }
        }

        $bottomEntryAngle = $null
        $bottomEntryPlaneSpeed = $null
        $mostNegativeAngle = Get-Minimum -Values @($cycleRows | ForEach-Object { $_.SwingAngleDeg })
        $selectedSwoopRadius = $null
        $zeroCross = $null
        $swoopExit = $null
        $zeroCrossRetention = $null
        $largestPlaneSpeedDrop = $null
        $bottomToZeroTicks = $null
        $zeroToExitTicks = $null
        $emergencySwoopCount = 0
        if ($null -ne $bottomEntry) {
            $bottomEntryAngle = if ($null -ne $bottomEntry.SwoopEntryAngleDeg) {
                $bottomEntry.SwoopEntryAngleDeg
            } else { $bottomEntry.SwingAngleDeg }
            $bottomEntryPlaneSpeed = if ($null -ne $bottomEntry.SwoopEntryPlaneSpeed) {
                $bottomEntry.SwoopEntryPlaneSpeed
            } else { $bottomEntry.SwingPlaneSpeed }

            $nextBottom = $rows | Where-Object {
                $_.ActivationId -eq $activationId -and $_.Tick -gt $bottomEntry.Tick -and
                ($_.Phase -eq 'BOTTOM' -or $_.AssistPhase -eq 'BOTTOM') -and
                $_.SwoopEntryPlaneSpeed -ne $bottomEntry.SwoopEntryPlaneSpeed
            } | Select-Object -First 1
            $swoopRows = @($rows | Where-Object {
                $_.ActivationId -eq $activationId -and $_.Tick -ge $bottomEntry.Tick -and
                ($null -eq $nextBottom -or $_.Tick -lt $nextBottom.Tick)
            })
            $selectedSwoopRadius = Get-Minimum -Values @($swoopRows | Where-Object {
                $null -ne $_.SwoopRadius -and $_.SwoopRadius -gt 0.0
            } | ForEach-Object { $_.SwoopRadius })
            $zeroCross = $swoopRows | Where-Object {
                $null -ne $_.SwingAngleDeg -and $_.SwingAngleDeg -ge 0.0
            } | Select-Object -First 1
            if ($null -ne $zeroCross) {
                $bottomToZeroTicks = [int64]$zeroCross.Tick - [int64]$bottomEntry.Tick
                if ($null -ne $bottomEntryPlaneSpeed -and [Math]::Abs([double]$bottomEntryPlaneSpeed) -gt 0.000001 -and
                    $null -ne $zeroCross.SwingPlaneSpeed) {
                    $zeroCrossRetention = [double]$zeroCross.SwingPlaneSpeed / [double]$bottomEntryPlaneSpeed
                }
                $swoopExit = $swoopRows | Where-Object {
                    $_.Tick -ge $zeroCross.Tick -and $_.Phase -eq 'ASCEND' -and
                    $null -ne $_.SwoopActive -and $_.SwoopActive -eq 0
                } | Select-Object -First 1
                if ($null -ne $swoopExit) {
                    $zeroToExitTicks = [int64]$swoopExit.Tick - [int64]$zeroCross.Tick
                }
            }

            $previousPlaneSpeed = $null
            $largestDropValue = 0.0
            foreach ($swoopRow in $swoopRows) {
                if ($null -eq $swoopRow.SwingPlaneSpeed) { continue }
                if ($null -ne $previousPlaneSpeed) {
                    $drop = [double]$previousPlaneSpeed - [double]$swoopRow.SwingPlaneSpeed
                    if ($drop -gt $largestDropValue) { $largestDropValue = $drop }
                }
                $previousPlaneSpeed = $swoopRow.SwingPlaneSpeed
            }
            $largestPlaneSpeedDrop = $largestDropValue

            $endTick = if ($null -eq $nextBottom) { [int64]::MaxValue } else { [int64]$nextBottom.Tick }
            $emergencySwoopCount = @($eventRows | Where-Object {
                (Get-TextValue -Row $_ -Name 'event_type') -eq 'SWOOP_EMERGENCY' -and
                (Get-NumberValue -Row $_ -Name 'activation_id') -eq $activationId -and
                (Get-NumberValue -Row $_ -Name 'tick') -ge $bottomEntry.Tick -and
                (Get-NumberValue -Row $_ -Name 'tick') -lt $endTick
            }).Count
        }

        $cycles += [pscustomobject][ordered]@{
            capture_cycle_index = [int64]$captureCycleIndex
            activation_id = if ($null -eq $activationId) { 0 } else { [int64]$activationId }
            controller_cycle_id = $controllerCycleId
            # Compatibility alias: cycle_id is still the controller value.
            cycle_id = $controllerCycleId
            start_tick = [int64]$cycleFirst.Tick
            end_tick = [int64]$cycleLast.Tick
            duration_seconds = ([double]($cycleLast.Tick - $cycleFirst.Tick)) / $physicsTickRate
            sample_count = $cycleRows.Count
            apex_y = $apexY
            low_y = $lowY
            vertical_excursion = if ($null -ne $apexY -and $null -ne $lowY) { [double]$apexY - $lowY } else { $null }
            minimum_clearance = $minClearance
            apex_horizontal_speed = $apexSpeed
            bottom_horizontal_speed = $bottomSpeed
            bottom_apex_speed_ratio = $ratio
            forward_displacement = $forwardDisplacement
            bottom_entry_tick = if ($null -eq $bottomEntry) { $null } else { [int64]$bottomEntry.Tick }
            reversal_start = $reversalDefinition
            reversal_status = $reversalStatus
            reversal_ticks = $reversalTicks
            reversal_seconds = if ($null -eq $reversalTicks) { $null } else { [double]$reversalTicks / $physicsTickRate }
            assist_energy_at_bottom_entry = if ($null -eq $bottomEntry) { $null } else { $bottomEntry.AssistEnergy }
            bottom_entry_angle_deg = $bottomEntryAngle
            bottom_entry_plane_speed = $bottomEntryPlaneSpeed
            most_negative_angle_deg = $mostNegativeAngle
            selected_swoop_radius = $selectedSwoopRadius
            zero_cross_tick = if ($null -eq $zeroCross) { $null } else { [int64]$zeroCross.Tick }
            zero_cross_plane_speed = if ($null -eq $zeroCross) { $null } else { $zeroCross.SwingPlaneSpeed }
            zero_cross_horizontal_speed = if ($null -eq $zeroCross) { $null } else { $zeroCross.HorizontalSpeed }
            zero_cross_retention = $zeroCrossRetention
            swoop_exit_tick = if ($null -eq $swoopExit) { $null } else { [int64]$swoopExit.Tick }
            swoop_exit_angle_deg = if ($null -eq $swoopExit) { $null } else { $swoopExit.SwingAngleDeg }
            swoop_exit_plane_speed = if ($null -eq $swoopExit) { $null } else { $swoopExit.SwingPlaneSpeed }
            emergency_swoop_count = $emergencySwoopCount
            largest_single_sample_plane_speed_drop = $largestPlaneSpeedDrop
            ticks_bottom_entry_to_zero_cross = $bottomToZeroTicks
            ticks_zero_cross_to_swoop_exit = $zeroToExitTicks
        }
    }

    $speedValues = @($rows | ForEach-Object { $_.TotalSpeed })
    $horizontalSpeedValues = @($rows | ForEach-Object { $_.HorizontalSpeed })
    $yValues = @($rows | ForEach-Object { $_.PosY })
    $clearanceValues = @($rows | ForEach-Object { Get-RowClearance $_ })
    $bottomClearanceValues = @($rows | Where-Object { $_.Phase -eq 'BOTTOM' -or $_.AssistPhase -eq 'BOTTOM' } | ForEach-Object { Get-RowClearance $_ })
    $averageApexSpeed = Get-Mean -Values @($cycles | ForEach-Object { $_.apex_horizontal_speed })
    $averageBottomSpeed = Get-Mean -Values @($cycles | ForEach-Object { $_.bottom_horizontal_speed })
    $aggregateRatio = if ($null -ne $averageApexSpeed -and [Math]::Abs([double]$averageApexSpeed) -gt 0.000001 -and $null -ne $averageBottomSpeed) { [double]$averageBottomSpeed / [double]$averageApexSpeed } else { $null }

    $phaseNames = @('LAUNCH', 'ASCEND', 'APEX', 'DESCEND', 'BOTTOM', 'NONE')
    $phaseMetrics = [ordered]@{}
    foreach ($phaseName in $phaseNames) {
        $phaseRows = @($rows | Where-Object { $_.Phase -eq $phaseName })
        $phaseCount = $phaseRows.Count
        $phaseMetrics[$phaseName] = [ordered]@{
            sample_count = $phaseCount
            duration_seconds = if ($phaseCount -eq 0) { 0.0 } else { [double]$phaseCount / $sampleRate }
            percent_of_samples = if ($rows.Count -eq 0) { 0.0 } else { 100.0 * $phaseCount / $rows.Count }
        }
    }

    $analysisRepoSha = ''
    try { $analysisRepoSha = (& git -C $repoRoot rev-parse HEAD 2>$null | Select-Object -First 1).ToString().Trim() } catch { $analysisRepoSha = '' }
    if ([string]::IsNullOrWhiteSpace($analysisRepoSha)) { $analysisRepoSha = 'not-recorded' }
    $captureId = if ($null -ne $metadata -and $null -ne $metadata.capture_id) { [int64]$metadata.capture_id } else { [int64]$first.CaptureId }
    $backendNames = @($rows | Select-Object -ExpandProperty Backend -Unique | Where-Object { -not [string]::IsNullOrWhiteSpace($_) })
    $backend = if ($backendNames.Count -eq 0) { 'unknown' } else { $backendNames -join '/' }
    $startedTime = if ($null -ne $metadata -and $null -ne $metadata.started_time) { [string]$metadata.started_time } else { $null }
    $sampleTickInterval = if ($null -ne $metadata -and $null -ne $metadata.sample_tick_interval) { [int]$metadata.sample_tick_interval } else { 2 }
    $runtimeBuildId = if ($null -ne $metadata -and $null -ne $metadata.runtime_build_id -and
                          -not [string]::IsNullOrWhiteSpace([string]$metadata.runtime_build_id)) {
        [string]$metadata.runtime_build_id
    } else { 'not-recorded' }

    $summaryObject = [ordered]@{
        schema_version = 3
        capture = [ordered]@{
            capture_id = $captureId
            source_directory = (Resolve-Path -LiteralPath $inputDirectory).Path
            packaged_directory = (Resolve-Path -LiteralPath $outputDirectory).Path
            started_tick = [int64]$first.Tick
            ended_tick = [int64]$last.Tick
            started_time = $startedTime
            analysis_repo_sha = $analysisRepoSha
            runtime_build_id = $runtimeBuildId
            backend = $backend
            source = 'client'
            development_only = $true
            physics_tick_rate_hz = $physicsTickRate
            sample_rate_hz = $sampleRate
            sample_tick_interval = $sampleTickInterval
            sample_count = $rows.Count
            event_count = $eventRows.Count
            cycle_identity = if ($captureIdentityAvailable) { 'capture_local' } else { 'legacy_controller_cycle_id' }
        }
        overall = [ordered]@{
            duration_seconds = $durationSeconds
            sample_count = $rows.Count
            cycle_count = $cycles.Count
            total_displacement = $totalDisplacement
            horizontal_displacement = $horizontalDisplacement
            net_altitude_change = $deltaY
        }
        speed = [ordered]@{
            average_speed = Get-Mean -Values $speedValues
            peak_speed = Get-Maximum -Values $speedValues
            average_horizontal_speed = Get-Mean -Values $horizontalSpeedValues
            peak_horizontal_speed = Get-Maximum -Values $horizontalSpeedValues
            average_apex_speed = $averageApexSpeed
            average_bottom_speed = $averageBottomSpeed
            bottom_apex_speed_ratio = $aggregateRatio
        }
        arc = [ordered]@{
            min_y = Get-Minimum -Values $yValues
            max_y = Get-Maximum -Values $yValues
            vertical_excursion = if ($yValues.Count -gt 0) { (Get-Maximum -Values $yValues) - (Get-Minimum -Values $yValues) } else { $null }
            average_cycle_vertical_excursion = Get-Mean -Values @($cycles | ForEach-Object { $_.vertical_excursion })
        }
        clearance = [ordered]@{
            minimum_clearance = Get-Minimum -Values $clearanceValues
            average_bottom_clearance = Get-Mean -Values $bottomClearanceValues
            clearance_source = 'minimum of current_ground_clearance and ahead_ground_clearance when present'
        }
        phase_metrics = $phaseMetrics
        consistency = [ordered]@{
            cycle_duration_stddev = Get-StdDev -Values @($cycles | ForEach-Object { $_.duration_seconds })
            apex_y_stddev = Get-StdDev -Values @($cycles | ForEach-Object { $_.apex_y })
            low_y_stddev = Get-StdDev -Values @($cycles | ForEach-Object { $_.low_y })
            bottom_speed_stddev = Get-StdDev -Values @($cycles | ForEach-Object { $_.bottom_horizontal_speed })
        }
        cycles = @($cycles)
        definitions = [ordered]@{
            duration = 'tick span divided by 30 physics ticks per second'
            phase_duration = 'sample count divided by nominal 15 samples per second'
            cycle_duration = 'first to last telemetry tick with the same capture_cycle_index, divided by 30'
            apex = 'highest sampled pos_y in the cycle'
            low = 'lowest sampled pos_y in the cycle'
            forward_displacement = 'horizontal displacement projected onto the first sampled horizontal intent; horizontal distance if no intent is available'
            minimum_clearance = 'minimum valid value across current and ahead terrain-clearance copies'
            bottom_speed = 'mean horizontal speed during BOTTOM samples, falling back to the lowest sampled row'
            reversal = 'ticks from BOTTOM entry, or strongest descending sample when no BOTTOM row exists, to the first positive vertical speed or ASCEND sample within the same capture_cycle_index; null when not observed'
            swoop = 'swoop metrics begin at this capture cycle bottom entry and may continue into the next controller cycle until active early-ASCEND redirection exits'
            zero_cross_retention = 'first sampled non-negative swing-plane speed divided by the controller-recorded BOTTOM-entry plane speed'
            standard_deviation = 'population standard deviation across captured cycles'
        }
    }

    Write-LineSvg -Path (Join-Path $outputDirectory 'height.svg') -Rows $rows -Property 'PosY' -Title 'Height vs motion tick' -Color '#8be28b'
    Write-LineSvg -Path (Join-Path $outputDirectory 'speed.svg') -Rows $rows -Property 'TotalSpeed' -Title 'Speed vs motion tick' -Color '#ffb454'
    Write-LineSvg -Path (Join-Path $outputDirectory 'vertical-velocity.svg') -Rows $rows -Property 'VerticalSpeed' -Title 'Vertical velocity vs motion tick' -Color '#d59cff'
    Write-LineSvg -Path (Join-Path $outputDirectory 'clearance.svg') -Rows $rows -Property 'CurrentClearance' -Title 'Current ground clearance vs motion tick' -Color '#55c7ff'
    Write-TopDownSvg -Path (Join-Path $outputDirectory 'path-topdown.svg') -Rows $rows

    $summaryText = New-Object System.Text.StringBuilder
    [void]$summaryText.AppendLine('WEBSWING MANUAL CAPTURE')
    [void]$summaryText.AppendLine('')
    [void]$summaryText.AppendLine(('Capture: CAP {0:D3}' -f $captureId))
    [void]$summaryText.AppendLine(('Analysis repo SHA: {0}' -f $analysisRepoSha))
    [void]$summaryText.AppendLine(('Runtime build ID: {0}' -f $runtimeBuildId))
    [void]$summaryText.AppendLine(('Backend: {0}' -f $backend))
    [void]$summaryText.AppendLine('Animation mode if detectable: not recorded; anim_phase and segment IDs are in telemetry')
    [void]$summaryText.AppendLine(('Duration: {0} s' -f (Format-Value $durationSeconds 3)))
    [void]$summaryText.AppendLine(('Samples: {0}' -f $rows.Count))
    [void]$summaryText.AppendLine(('Cycles: {0}' -f $cycles.Count))
    [void]$summaryText.AppendLine('')
    [void]$summaryText.AppendLine('TRAVEL')
    [void]$summaryText.AppendLine(('Total displacement: {0}' -f (Format-Value $totalDisplacement 3)))
    [void]$summaryText.AppendLine(('Horizontal displacement: {0}' -f (Format-Value $horizontalDisplacement 3)))
    [void]$summaryText.AppendLine(('Net altitude: {0}' -f (Format-Value $deltaY 3)))
    [void]$summaryText.AppendLine('')
    [void]$summaryText.AppendLine('SPEED')
    [void]$summaryText.AppendLine(('Average speed: {0}' -f (Format-Value (Get-Mean $speedValues) 3)))
    [void]$summaryText.AppendLine(('Peak speed: {0}' -f (Format-Value (Get-Maximum $speedValues) 3)))
    [void]$summaryText.AppendLine(('Average horizontal speed: {0}' -f (Format-Value (Get-Mean $horizontalSpeedValues) 3)))
    [void]$summaryText.AppendLine(('Peak horizontal speed: {0}' -f (Format-Value (Get-Maximum $horizontalSpeedValues) 3)))
    [void]$summaryText.AppendLine(('Average apex speed: {0}' -f (Format-Value $averageApexSpeed 3)))
    [void]$summaryText.AppendLine(('Average bottom speed: {0}' -f (Format-Value $averageBottomSpeed 3)))
    [void]$summaryText.AppendLine(('Bottom/apex ratio: {0}' -f (Format-Value $aggregateRatio 3)))
    [void]$summaryText.AppendLine('')
    [void]$summaryText.AppendLine('ARC')
    [void]$summaryText.AppendLine(('Minimum Y: {0}' -f (Format-Value (Get-Minimum $yValues) 3)))
    [void]$summaryText.AppendLine(('Maximum Y: {0}' -f (Format-Value (Get-Maximum $yValues) 3)))
    [void]$summaryText.AppendLine(('Average vertical excursion: {0}' -f (Format-Value $summaryObject.arc.average_cycle_vertical_excursion 3)))
    [void]$summaryText.AppendLine(('Average clearance: {0}' -f (Format-Value (Get-Mean $clearanceValues) 3)))
    [void]$summaryText.AppendLine(('Average bottom clearance: {0}' -f (Format-Value (Get-Mean $bottomClearanceValues) 3)))
    [void]$summaryText.AppendLine('')
    [void]$summaryText.AppendLine('PHASE DISTRIBUTION')
    foreach ($phaseName in $phaseNames) {
        $phaseMetric = $phaseMetrics[$phaseName]
        [void]$summaryText.AppendLine(('{0}: {1} samples, {2} s, {3}' -f $phaseName, $phaseMetric.sample_count, (Format-Value $phaseMetric.duration_seconds 3), (Format-Percent $phaseMetric.percent_of_samples)))
    }
    [void]$summaryText.AppendLine('')
    [void]$summaryText.AppendLine('SWOOP TABLE')
    [void]$summaryText.AppendLine('CaptureCycle | Energy | EntryAngle | EntryPlane | Radius | ZeroTick | ZeroPlane | ZeroHoriz | Retain | ExitAngle | ExitPlane | MinClear | Emerg')
    [void]$summaryText.AppendLine('-------------|--------|------------|------------|--------|----------|-----------|-----------|--------|-----------|-----------|----------|------')
    foreach ($cycle in $cycles | Where-Object { $null -ne $_.bottom_entry_angle_deg }) {
        [void]$summaryText.AppendLine(('{0,12} | {1,6} | {2,10} | {3,10} | {4,6} | {5,8} | {6,9} | {7,9} | {8,6} | {9,9} | {10,9} | {11,8} | {12,5}' -f
            $cycle.capture_cycle_index,
            (Format-Value $cycle.assist_energy_at_bottom_entry 2),
            (Format-Value $cycle.bottom_entry_angle_deg 2),
            (Format-Value $cycle.bottom_entry_plane_speed 2),
            (Format-Value $cycle.selected_swoop_radius 2),
            $(if ($null -eq $cycle.zero_cross_tick) { 'n/a' } else { $cycle.zero_cross_tick }),
            (Format-Value $cycle.zero_cross_plane_speed 2),
            (Format-Value $cycle.zero_cross_horizontal_speed 2),
            (Format-Percent $(if ($null -eq $cycle.zero_cross_retention) { $null } else { 100.0 * $cycle.zero_cross_retention })),
            (Format-Value $cycle.swoop_exit_angle_deg 2),
            (Format-Value $cycle.swoop_exit_plane_speed 2),
            (Format-Value $cycle.minimum_clearance 2),
            $cycle.emergency_swoop_count))
    }
    [void]$summaryText.AppendLine('')
    [void]$summaryText.AppendLine('CONSISTENCY')
    [void]$summaryText.AppendLine(('cycle duration stddev: {0}' -f (Format-Value $summaryObject.consistency.cycle_duration_stddev 3)))
    [void]$summaryText.AppendLine(('apex Y stddev: {0}' -f (Format-Value $summaryObject.consistency.apex_y_stddev 3)))
    [void]$summaryText.AppendLine(('low Y stddev: {0}' -f (Format-Value $summaryObject.consistency.low_y_stddev 3)))
    [void]$summaryText.AppendLine(('bottom speed stddev: {0}' -f (Format-Value $summaryObject.consistency.bottom_speed_stddev 3)))
    [void]$summaryText.AppendLine('')
    [void]$summaryText.AppendLine('CYCLE TABLE (capture-local identity)')
    [void]$summaryText.AppendLine('CaptureCycle | Activation | ControllerCycle | Duration | ApexY | LowY | Excursion | MinClear | ApexSpd | BottomSpd | Bottom/Apex | Forward')
    [void]$summaryText.AppendLine('-------------|------------|-----------------|----------|-------|------|-----------|----------|----------|------------|-------------|--------')
    if ($cycles.Count -eq 0) {
        [void]$summaryText.AppendLine('No swing cycles with capture_cycle_index > 0.')
    } else {
        foreach ($cycle in $cycles) {
            [void]$summaryText.AppendLine(('{0,12} | {1,10} | {2,15} | {3,8} | {4,5} | {5,5} | {6,9} | {7,8} | {8,8} | {9,10} | {10,11} | {11,8}' -f
                $cycle.capture_cycle_index,
                $cycle.activation_id,
                $cycle.controller_cycle_id,
                (Format-Value $cycle.duration_seconds 3),
                (Format-Value $cycle.apex_y 2),
                (Format-Value $cycle.low_y 2),
                (Format-Value $cycle.vertical_excursion 2),
                (Format-Value $cycle.minimum_clearance 2),
                (Format-Value $cycle.apex_horizontal_speed 2),
                (Format-Value $cycle.bottom_horizontal_speed 2),
                (Format-Value $cycle.bottom_apex_speed_ratio 2),
                (Format-Value $cycle.forward_displacement 2)))
        }
    }
    [void]$summaryText.AppendLine('')
    [void]$summaryText.AppendLine('Definitions: phase durations use nominal 15 Hz samples; capture cycles use capture_cycle_index; cycle_id/controller_cycle_id retain the controller value; reversal never crosses a capture cycle and is null when not observed.')
    if (-not $captureIdentityAvailable) {
        [void]$summaryText.AppendLine('Warning: this is a legacy capture without capture-local identity fields; capture_cycle_index falls back to controller cycle_id and cannot disambiguate reused controller ids.')
    }
    [void]$summaryText.AppendLine(('Raw capture: {0}' -f $inputDirectory))
    [void]$summaryText.AppendLine(('Packaged artifacts: {0}' -f $outputDirectory))
    [IO.File]::WriteAllText((Join-Path $outputDirectory 'summary.txt'), $summaryText.ToString(), (New-Object System.Text.UTF8Encoding($false)))

    $summaryJson = $summaryObject | ConvertTo-Json -Depth 15
    [IO.File]::WriteAllText((Join-Path $outputDirectory 'summary.json'), $summaryJson, (New-Object System.Text.UTF8Encoding($false)))

    if ($GoldenCycle -gt 0) {
        $goldenRows = @($rows | Where-Object { $_.CaptureCycleIndex -eq $GoldenCycle })
        if ($goldenRows.Count -eq 0) { throw "Golden cycle $GoldenCycle was not present in the capture." }
        $goldenSamples = @($goldenRows | ForEach-Object {
            [ordered]@{
                tick = $_.Tick
                phase = $_.Phase
                phase_ticks = $_.PhaseTicks
                capture_cycle_index = $_.CaptureCycleIndex
                activation_id = $_.ActivationId
                controller_cycle_id = $_.ControllerCycleId
                cycle_id = $_.ControllerCycleId
                pos_x = $_.PosX; pos_y = $_.PosY; pos_z = $_.PosZ
                vel_x = $_.VelX; vel_y = $_.VelY; vel_z = $_.VelZ
                total_speed = $_.TotalSpeed; horizontal_speed = $_.HorizontalSpeed
                swing_plane_speed = $_.SwingPlaneSpeed; swing_angle_deg = $_.SwingAngleDeg
                forward_speed_along_intent = $_.ForwardSpeedAlongIntent
                swoop_active = $_.SwoopActive; swoop_radius = $_.SwoopRadius
                swoop_entry_plane_speed = $_.SwoopEntryPlaneSpeed
                swoop_entry_angle_deg = $_.SwoopEntryAngleDeg
                swoop_emergency_count = $_.SwoopEmergencyCount
                current_ground_clearance = $_.CurrentClearance
                ahead_ground_clearance = $_.AheadClearance
                low_point_y = $_.LowPointY
                intent_x = $_.IntentX; intent_y = $_.IntentY; intent_z = $_.IntentZ
            }
        })
        $goldenObject = [ordered]@{
            schema_version = 3
            source_capture_id = $captureId
            capture_cycle_index = $GoldenCycle
            cycle = @($cycles | Where-Object { $_.capture_cycle_index -eq $GoldenCycle } | Select-Object -First 1)
            samples = $goldenSamples
            definitions = 'Reference telemetry only; this artifact does not tune or optimize Web Swing.'
        }
        [IO.File]::WriteAllText((Join-Path $outputDirectory 'golden-swing.json'), ($goldenObject | ConvertTo-Json -Depth 12), (New-Object System.Text.UTF8Encoding($false)))
    }

    $artifactList = @('telemetry.csv', 'events.csv', 'metadata.json', 'summary.txt', 'summary.json', 'height.svg', 'speed.svg', 'vertical-velocity.svg', 'clearance.svg', 'path-topdown.svg')
    if ($GoldenCycle -gt 0) { $artifactList += 'golden-swing.json' }
    $result = [ordered]@{
        passed = $true
        capture_id = $captureId
        input_directory = (Resolve-Path -LiteralPath $inputDirectory).Path
        output_directory = (Resolve-Path -LiteralPath $outputDirectory).Path
        summary_json = (Join-Path $outputDirectory 'summary.json')
        summary_text = (Join-Path $outputDirectory 'summary.txt')
        telemetry_samples = $rows.Count
        cycle_count = $cycles.Count
        event_count = $eventRows.Count
        artifacts = $artifactList
    }
} catch {
    $result = [ordered]@{
        passed = $false
        error = $_.Exception.Message
        input_directory = $InputPath
    }
}

if ($Json) {
    $result | ConvertTo-Json -Depth 15
} elseif ($result.passed) {
    Write-Host ('WEB SWING CAPTURE PACKAGED - CAP {0:D3}' -f $result.capture_id)
    Write-Host ('Samples: {0}; cycles: {1}; events: {2}' -f $result.telemetry_samples, $result.cycle_count, $result.event_count)
    Write-Host ('Capture folder: {0}' -f $result.output_directory)
    Write-Host ('Summary: {0}' -f $result.summary_text)
} else {
    Write-Host ('WEB SWING CAPTURE ERROR - {0}' -f $result.error)
}

if ($result.passed) { exit 0 } else { exit 1 }
