$ErrorActionPreference = "Stop"

$runtimeDirectory = Split-Path -Parent $PSScriptRoot
$composeFile = Join-Path $runtimeDirectory "docker-compose.yml"
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

function Assert-FieldIn {
    param(
        [Parameter(Mandatory)]$Response,
        [Parameter(Mandatory)][string]$Field,
        [Parameter(Mandatory)][object[]]$Expected,
        [Parameter(Mandatory)][string]$Step
    )

    $property = $Response.PSObject.Properties[$Field]
    if ($null -eq $property -or $property.Value -notin $Expected) {
        throw "$Step returned an unexpected '$Field' value."
    }
}

function Assert-NonEmptyField {
    param(
        [Parameter(Mandatory)]$Response,
        [Parameter(Mandatory)][string]$Field,
        [Parameter(Mandatory)][string]$Step
    )

    $property = $Response.PSObject.Properties[$Field]
    if ($null -eq $property -or [string]::IsNullOrWhiteSpace([string]$property.Value)) {
        throw "$Step did not return a non-empty '$Field'."
    }
}

function Invoke-MongoScript {
    param(
        [Parameter(Mandatory)][string]$Script,
        [switch]$IgnoreErrors
    )

    $mongoCommand = 'exec mongosh --quiet --username "$MONGO_INITDB_ROOT_USERNAME" --password "$MONGO_INITDB_ROOT_PASSWORD" --authenticationDatabase admin "$MONGO_INITDB_DATABASE" --file /dev/stdin'
    $Script | & docker compose --env-file $envFile -f $composeFile `
        exec -T mongodb sh -lc $mongoCommand | Out-Null
    if ($LASTEXITCODE -eq 0) { return $true }

    if ($IgnoreErrors) {
        Write-Warning "Smoke fixture cleanup failed; inspect the MongoDB container before rerunning."
        return $false
    }
    throw "MongoDB smoke fixture command failed."
}

$cleanupScript = @'
const database = db.getSiblingDB(process.env.MONGO_INITDB_DATABASE || "securezone");
database.qr_checkins.deleteMany({$or:[{employeeId:"SMOKE-EMPLOYEE"},{zoneId:"SMOKE-ZONE"},{scannedByUserId:"SMOKE-SCANNER"}]});
database.presence_sessions.deleteMany({$or:[{employeeId:"SMOKE-EMPLOYEE"},{zoneId:"SMOKE-ZONE"}]});
database.camera_object_tracks.deleteMany({cameraId:"CAM-SMOKE"});
database.track_identity_bindings.deleteMany({$or:[{cameraId:"CAM-SMOKE"},{employeeId:"SMOKE-EMPLOYEE"}]});
database.alarms.deleteMany({$or:[{zoneId:"SMOKE-ZONE"},{employeeId:"SMOKE-EMPLOYEE"},{machineId:"SMOKE-MACHINE"},{trackId:{$regex:"^CAM-SMOKE:"}}]});
database.access_policies.deleteMany({policyId:"SMOKE-POLICY"});
database.zones.deleteMany({zoneId:"SMOKE-ZONE"});
database.machines.deleteMany({machineId:"SMOKE-MACHINE"});
database.app_users.deleteMany({userId:"SMOKE-SCANNER"});
database.employees.deleteMany({employeeId:"SMOKE-EMPLOYEE"});
'@

$setupScript = $cleanupScript + @'

database.employees.insertOne({employeeId:"SMOKE-EMPLOYEE",fullName:"Smoke Test Employee",department:"Testing",roles:["maintenance"],status:"active",qrTokenHash:"smoke-test-token-hash"});
database.app_users.insertOne({userId:"SMOKE-SCANNER",username:"smoke-scanner",role:"scanner",status:"active"});
database.machines.insertOne({machineId:"SMOKE-MACHINE",name:"Smoke Test Machine",status:"stopped",updatedAt:new Date()});
database.zones.insertOne({zoneId:"SMOKE-ZONE",name:"Smoke Test Dangerous Zone",cameraId:"CAM-SMOKE",type:"dangerous",status:"active",relatedMachineId:"SMOKE-MACHINE",xprotectEventName:"SecureZone.OpenSDK.WiseAI.LineCrossing.Smoke.State-2"});
database.access_policies.insertOne({policyId:"SMOKE-POLICY",zoneId:"SMOKE-ZONE",allowedRoles:["maintenance"],machineStatesAllowed:["stopped","maintenance"],timeWindows:[]});
'@

$apiPort = Get-DotEnvValue "SECUREZONE_API_HOST_PORT"
if ([string]::IsNullOrWhiteSpace($apiPort)) { $apiPort = "8080" }

$apiKey = Get-DotEnvValue "SECUREZONE_XPROTECT_API_KEY"
if ([string]::IsNullOrWhiteSpace($apiKey) -or $apiKey.StartsWith("<")) {
    throw "SECUREZONE_XPROTECT_API_KEY must be set to a real local development value in deployment/runtime/.env."
}

$baseUri = "http://127.0.0.1:$apiPort"
$headers = @{ "X-SecureZone-Api-Key" = $apiKey }
$eventName = "SecureZone.OpenSDK.WiseAI.LineCrossing.Smoke.State-2"
$sourceName = "SecureZone Smoke Camera"
$cameraId = "CAM-SMOKE"
$unknownObjectId = "SMOKE-UNKNOWN-OBJECT"
$authorizedObjectId = "SMOKE-AUTHORIZED-OBJECT"
$flowSucceeded = $false
$cleanupSucceeded = $false

try {
    Write-Host "[1/7] Health check"
    $health = Invoke-RestMethod -Method Get -Uri "$baseUri/health"
    Assert-FieldEquals $health "status" "ok" "Health check"

    Write-Host "Preparing isolated MongoDB smoke fixtures..."
    Invoke-MongoScript -Script $setupScript | Out-Null

    Write-Host "[2/7] Unknown camera object -> violation"
    $unknownEnter = @{
        eventId = "smoke-unknown-enter-$([guid]::NewGuid().ToString('N'))"
        eventName = $eventName
        sourceName = $sourceName
        receivedAt = [DateTimeOffset]::UtcNow.ToString("o")
        cameraId = $cameraId
        objectId = $unknownObjectId
        action = "enter"
    } | ConvertTo-Json -Compress
    $unknownEnterDecision = Invoke-RestMethod -Method Post `
        -Uri "$baseUri/api/xprotect/line-crossing" -Headers $headers `
        -ContentType "application/json" -Body $unknownEnter
    Assert-FieldEquals $unknownEnterDecision "accepted" $true "Unknown-object LineCrossing"
    Assert-FieldEquals $unknownEnterDecision "decision" "violation" "Unknown-object LineCrossing"

    Write-Host "[3/7] Unknown camera object exit -> alarm cleared"
    $unknownExit = @{
        eventId = "smoke-unknown-exit-$([guid]::NewGuid().ToString('N'))"
        eventName = $eventName
        sourceName = $sourceName
        receivedAt = [DateTimeOffset]::UtcNow.ToString("o")
        cameraId = $cameraId
        objectId = $unknownObjectId
        action = "exit"
    } | ConvertTo-Json -Compress
    $unknownExitDecision = Invoke-RestMethod -Method Post `
        -Uri "$baseUri/api/xprotect/line-crossing" -Headers $headers `
        -ContentType "application/json" -Body $unknownExit
    Assert-FieldEquals $unknownExitDecision "accepted" $true "Unknown-object exit"
    Assert-FieldEquals $unknownExitDecision "decision" "cleared" "Unknown-object exit"

    Write-Host "[4/7] Recent Human camera observation"
    $observation = @{
        cameraId = $cameraId
        objectId = $authorizedObjectId
        objectType = "Human"
        observedAt = [DateTimeOffset]::UtcNow.ToString("o")
    } | ConvertTo-Json -Compress
    $observationResult = Invoke-RestMethod -Method Post `
        -Uri "$baseUri/api/xprotect/object-observations" -Headers $headers `
        -ContentType "application/json" -Body $observation
    Assert-FieldEquals $observationResult "accepted" $true "Human observation"
    Assert-FieldEquals $observationResult "status" "observed" "Human observation"

    Write-Host "[5/7] QR check-in -> camera object identity binding"
    $checkIn = @{
        employeeId = "SMOKE-EMPLOYEE"
        zoneId = "SMOKE-ZONE"
        scannedByUserId = "SMOKE-SCANNER"
        cameraId = $cameraId
    } | ConvertTo-Json -Compress
    $checkInResult = Invoke-RestMethod -Method Post -Uri "$baseUri/api/qr/check-in" `
        -ContentType "application/json" -Body $checkIn
    Assert-FieldEquals $checkInResult "accepted" $true "QR check-in"
    Assert-FieldIn $checkInResult "status" @("started", "extended", "already_active") "QR check-in"
    Assert-FieldEquals $checkInResult "objectId" $authorizedObjectId "QR check-in"
    Assert-NonEmptyField $checkInResult "bindingId" "QR check-in"

    Write-Host "[6/7] Bound camera object -> allowed"
    $authorizedEvent = @{
        eventId = "smoke-authorized-enter-$([guid]::NewGuid().ToString('N'))"
        eventName = $eventName
        sourceName = $sourceName
        receivedAt = [DateTimeOffset]::UtcNow.ToString("o")
        cameraId = $cameraId
        objectId = $authorizedObjectId
        action = "enter"
    } | ConvertTo-Json -Compress
    $authorizedDecision = Invoke-RestMethod -Method Post `
        -Uri "$baseUri/api/xprotect/line-crossing" -Headers $headers `
        -ContentType "application/json" -Body $authorizedEvent
    Assert-FieldEquals $authorizedDecision "accepted" $true "Authorized-object LineCrossing"
    Assert-FieldEquals $authorizedDecision "decision" "allowed" "Authorized-object LineCrossing"
    Assert-FieldEquals $authorizedDecision "zoneId" "SMOKE-ZONE" "Authorized-object LineCrossing"
    Assert-FieldEquals $authorizedDecision "employeeId" "SMOKE-EMPLOYEE" "Authorized-object LineCrossing"

    $flowSucceeded = $true
}
finally {
    Write-Host "[7/7] Cleanup"
    $cleanupSucceeded = Invoke-MongoScript -Script $cleanupScript -IgnoreErrors
}

if ($flowSucceeded -and -not $cleanupSucceeded) {
    throw "Smoke flow passed, but isolated fixture cleanup failed."
}

Write-Host "SecureZone runtime camera-identity smoke test passed."
