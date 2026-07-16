$ErrorActionPreference = "Stop"

$runtimeDirectory = Split-Path -Parent $PSScriptRoot
$envFile = Join-Path $runtimeDirectory ".env"

if (-not (Test-Path -LiteralPath $envFile -PathType Leaf)) {
    throw "Missing deployment/runtime/.env. Copy .env.example to .env and replace every placeholder."
}

function Get-DotEnvValue {
    param([Parameter(Mandatory)][string]$Name)

    foreach ($line in Get-Content -LiteralPath $envFile) {
        if ($line -match "^\s*$([regex]::Escape($Name))\s*=\s*(.*)\s*$") {
            $value = $Matches[1].Trim()
            if ($value.Length -ge 2 -and
                (($value.StartsWith('"') -and $value.EndsWith('"')) -or
                 ($value.StartsWith("'") -and $value.EndsWith("'")))) {
                return $value.Substring(1, $value.Length - 2)
            }
            return $value
        }
    }
    return $null
}

function Assert-FieldEquals {
    param(
        [Parameter(Mandatory)]$Response,
        [Parameter(Mandatory)][string]$Field,
        [Parameter(Mandatory)]$Expected,
        [Parameter(Mandatory)][string]$Step
    )

    $property = $Response.PSObject.Properties[$Field]
    if ($null -eq $property -or $property.Value -ne $Expected) {
        throw "$Step returned an unexpected '$Field' value."
    }
}

$apiPort = Get-DotEnvValue "SECUREZONE_API_HOST_PORT"
if ([string]::IsNullOrWhiteSpace($apiPort)) { $apiPort = "8080" }

$apiKey = Get-DotEnvValue "SECUREZONE_XPROTECT_API_KEY"
if ([string]::IsNullOrWhiteSpace($apiKey) -or $apiKey.StartsWith("<")) {
    throw "SECUREZONE_XPROTECT_API_KEY must be set to a real local development value in deployment/runtime/.env."
}

$baseUri = "http://127.0.0.1:$apiPort"
$headers = @{ "X-SecureZone-Api-Key" = $apiKey }
$eventName = "Channel.<int>.OpenSDK.WiseAI.LineCrossing.<int>.State-2"
$sourceName = "Hanwha Vision TNO-C4052T TEST-CAMERA - Camera 1"

Write-Host "[1/4] Health check"
$health = Invoke-RestMethod -Method Get -Uri "$baseUri/health"
Assert-FieldEquals $health "status" "ok" "Health check"

Write-Host "[2/4] Line crossing without QR presence -> violation"
$firstEvent = @{
    eventId = "smoke-before-$([guid]::NewGuid().ToString('N'))"
    eventName = $eventName
    sourceName = $sourceName
    receivedAt = [DateTimeOffset]::UtcNow.ToString("o")
} | ConvertTo-Json -Compress
$firstDecision = Invoke-RestMethod -Method Post -Uri "$baseUri/api/xprotect/line-crossing" `
    -Headers $headers -ContentType "application/json" -Body $firstEvent
Assert-FieldEquals $firstDecision "accepted" $true "Initial line crossing"
Assert-FieldEquals $firstDecision "decision" "violation" "Initial line crossing"

Write-Host "[3/4] QR check-in -> accepted"
$checkIn = @{
    employeeId = "EMP-001"
    zoneId = "ZONE-001"
    scannedByUserId = "APP-SCANNER-001"
} | ConvertTo-Json -Compress
$checkInResult = Invoke-RestMethod -Method Post -Uri "$baseUri/api/qr/check-in" `
    -ContentType "application/json" -Body $checkIn
Assert-FieldEquals $checkInResult "accepted" $true "QR check-in"

Write-Host "[4/4] Line crossing with active QR presence -> allowed"
$secondEvent = @{
    eventId = "smoke-after-$([guid]::NewGuid().ToString('N'))"
    eventName = $eventName
    sourceName = $sourceName
    receivedAt = [DateTimeOffset]::UtcNow.ToString("o")
} | ConvertTo-Json -Compress
$secondDecision = Invoke-RestMethod -Method Post -Uri "$baseUri/api/xprotect/line-crossing" `
    -Headers $headers -ContentType "application/json" -Body $secondEvent
Assert-FieldEquals $secondDecision "accepted" $true "Post-check-in line crossing"
Assert-FieldEquals $secondDecision "decision" "allowed" "Post-check-in line crossing"

Write-Host "SecureZone runtime smoke test passed."
