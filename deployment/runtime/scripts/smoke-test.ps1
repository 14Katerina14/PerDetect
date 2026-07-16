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

function Get-UtcTimestamp {
    return [DateTime]::UtcNow.ToString(
        "yyyy-MM-dd'T'HH:mm:ss.fff'Z'",
        [Globalization.CultureInfo]::InvariantCulture
    )
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

function Invoke-ScannerProvisioning {
    param([Parameter(Mandatory)][string]$Password)

    $Password | & docker compose --env-file $envFile -f $composeFile `
        exec -T securezone-api /app/SecureZone.ProvisionUser `
        --user-id SMOKE-SCANNER --username smoke-scanner --role scanner | Out-Null
    if ($LASTEXITCODE -ne 0) {
        throw "Scanner application-user provisioning failed."
    }
}

$cleanupScript = @'
const database = db.getSiblingDB(process.env.MONGO_INITDB_DATABASE || "securezone");
database.qr_checkins.deleteMany({$or:[{employeeId:"SMOKE-EMPLOYEE"},{zoneId:"SMOKE-ZONE"},{scannedByUserId:"SMOKE-SCANNER"}]});
database.presence_sessions.deleteMany({$or:[{employeeId:"SMOKE-EMPLOYEE"},{zoneId:"SMOKE-ZONE"}]});
database.camera_object_tracks.deleteMany({cameraId:"CAM-SMOKE"});
database.track_identity_bindings.deleteMany({$or:[{cameraId:"CAM-SMOKE"},{employeeId:"SMOKE-EMPLOYEE"}]});
database.alarms.deleteMany({$or:[{zoneId:"SMOKE-ZONE"},{employeeId:"SMOKE-EMPLOYEE"},{machineId:"SMOKE-MACHINE"},{trackId:{$regex:"^CAM-SMOKE:"}}]});
database.push_notification_deliveries.deleteMany({zoneId:"SMOKE-ZONE"});
database.push_subscriptions.deleteMany({subscriptionId:"SMOKE-MANAGER-SUBSCRIPTION"});
database.access_policies.deleteMany({policyId:"SMOKE-POLICY"});
database.zones.deleteMany({zoneId:"SMOKE-ZONE"});
database.machines.deleteMany({machineId:"SMOKE-MACHINE"});
database.app_users.deleteMany({userId:{$in:["SMOKE-SCANNER","SMOKE-MANAGER"]}});
database.employees.deleteMany({employeeId:"SMOKE-EMPLOYEE"});
'@

$setupScript = $cleanupScript + @'

database.employees.insertOne({employeeId:"SMOKE-EMPLOYEE",fullName:"Smoke Test Employee",department:"Testing",roles:["operator"],status:"active",qrTokenHash:"smoke-test-token-hash"});
database.machines.insertOne({machineId:"SMOKE-MACHINE",name:"Smoke Test Machine",status:"stopped",updatedAt:new Date()});
database.zones.insertOne({zoneId:"SMOKE-ZONE",name:"Smoke Test Dangerous Zone",cameraId:"CAM-SMOKE",type:"dangerous",status:"active",relatedMachineId:"SMOKE-MACHINE",xprotectEventName:"SecureZone.OpenSDK.WiseAI.LineCrossing.Smoke.State-2"});
database.access_policies.insertOne({policyId:"SMOKE-POLICY",zoneId:"SMOKE-ZONE",allowedRoles:["maintenance"],machineStatesAllowed:["stopped","maintenance"],timeWindows:[]});
database.app_users.insertOne({userId:"SMOKE-MANAGER",username:"smoke-manager",passwordHash:"not-used-by-smoke-test",role:"manager",status:"active"});
database.push_subscriptions.insertOne({subscriptionId:"SMOKE-MANAGER-SUBSCRIPTION",userId:"SMOKE-MANAGER",provider:"expo",deviceToken:"ExponentPushToken[smoke-manager]",status:"active",updatedAt:new Date()});
'@

$activeAlarmAssertion = @'
const database = db.getSiblingDB(process.env.MONGO_INITDB_DATABASE || "securezone");
const alarm = database.alarms.findOne({zoneId:"SMOKE-ZONE",trackId:"CAM-SMOKE:SMOKE-WORKER-OBJECT"});
if (!alarm) throw new Error("Expected a persisted smoke alarm.");
if (alarm.status !== "active" || alarm.stillInside !== true) throw new Error("Smoke alarm is not active.");
if (alarm.employeeId !== "SMOKE-EMPLOYEE") throw new Error("Smoke alarm is not linked to the QR employee.");
if (!String(alarm.reason).includes("role")) throw new Error("Smoke alarm does not contain the role denial reason.");
const created = database.push_notification_deliveries.find({alarmId:alarm.alarmId,title:"SecureZone access violation",recipientUserId:"SMOKE-MANAGER",status:"pending"}).toArray();
if (created.length !== 1) throw new Error("Expected exactly one pending manager violation notification.");
if (database.alarms.countDocuments({zoneId:"SMOKE-ZONE",status:"active"}) !== 1) throw new Error("Expected exactly one active smoke alarm.");
'@

$resolvedAlarmAssertion = @'
const database = db.getSiblingDB(process.env.MONGO_INITDB_DATABASE || "securezone");
const alarm = database.alarms.findOne({zoneId:"SMOKE-ZONE",trackId:"CAM-SMOKE:SMOKE-WORKER-OBJECT"});
if (!alarm) throw new Error("Expected the persisted smoke alarm.");
if (alarm.status !== "resolved" || alarm.stillInside !== false) throw new Error("Smoke alarm was not resolved.");
if (!(alarm.resolvedAt instanceof Date) || !(alarm.exitedAt instanceof Date)) throw new Error("Resolved smoke alarm has no exit timestamps.");
const createdCount = database.push_notification_deliveries.countDocuments({alarmId:alarm.alarmId,title:"SecureZone access violation",recipientUserId:"SMOKE-MANAGER"});
const cleared = database.push_notification_deliveries.find({alarmId:alarm.alarmId,title:"SecureZone violation cleared",recipientUserId:"SMOKE-MANAGER",status:"pending"}).toArray();
if (createdCount !== 1) throw new Error("Violation notification was duplicated.");
if (cleared.length !== 1) throw new Error("Expected exactly one pending manager clear notification.");
if (database.alarms.countDocuments({zoneId:"SMOKE-ZONE",status:{$in:["active","acknowledged"]}}) !== 0) throw new Error("An active smoke alarm remains after exit.");
'@

$apiPort = Get-DotEnvValue "SECUREZONE_API_HOST_PORT"
if ([string]::IsNullOrWhiteSpace($apiPort)) { $apiPort = "18080" }

$apiKey = Get-DotEnvValue "SECUREZONE_XPROTECT_API_KEY"
if ([string]::IsNullOrWhiteSpace($apiKey) -or $apiKey.StartsWith("<")) {
    throw "SECUREZONE_XPROTECT_API_KEY must be set to a real local development value in deployment/runtime/.env."
}

$baseUri = "http://127.0.0.1:$apiPort"
$headers = @{ "X-SecureZone-Api-Key" = $apiKey }
$eventName = "SecureZone.OpenSDK.WiseAI.LineCrossing.Smoke.State-2"
$sourceName = "SecureZone Smoke Camera"
$cameraId = "CAM-SMOKE"
$workerObjectId = "SMOKE-WORKER-OBJECT"
$flowSucceeded = $false
$cleanupSucceeded = $false

try {
    Write-Host "[1/10] Health and version check"
    $health = Invoke-RestMethod -Method Get -Uri "$baseUri/health"
    Assert-FieldEquals $health "status" "ok" "Health check"

    Write-Host "Preparing isolated MongoDB smoke fixtures..."
    Invoke-MongoScript -Script $setupScript | Out-Null

    $version = Invoke-RestMethod -Method Get -Uri "$baseUri/version"
    Assert-NonEmptyField $version "buildId" "Version check"

    Write-Host "[2/10] Provision scanner and obtain JWT"
    $scannerPassword = "Smoke-$([guid]::NewGuid().ToString('N'))"
    Invoke-ScannerProvisioning -Password $scannerPassword
    $loginBody = @{ username = "smoke-scanner"; password = $scannerPassword } |
        ConvertTo-Json -Compress
    $loginResult = Invoke-RestMethod -Method Post -Uri "$baseUri/api/auth/login" `
        -ContentType "application/json" -Body $loginBody
    Assert-NonEmptyField $loginResult "accessToken" "Scanner login"
    $scannerHeaders = @{ Authorization = "Bearer $($loginResult.accessToken)" }
    $scannerPassword = $null

    Write-Host "[3/10] Inject recent Human camera observation"
    $observation = @{
        cameraId = $cameraId
        objectId = $workerObjectId
        objectType = "Human"
        observedAt = Get-UtcTimestamp
    } | ConvertTo-Json -Compress
    $observationResult = Invoke-RestMethod -Method Post `
        -Uri "$baseUri/api/xprotect/object-observations" -Headers $headers `
        -ContentType "application/json" -Body $observation
    Assert-FieldEquals $observationResult "accepted" $true "Human observation"
    Assert-FieldEquals $observationResult "status" "observed" "Human observation"

    Write-Host "[4/10] Authenticated QR check-in -> Human/employee binding"
    $checkIn = @{
        employeeId = "SMOKE-EMPLOYEE"
        zoneId = "SMOKE-ZONE"
        cameraId = $cameraId
    } | ConvertTo-Json -Compress
    $checkInResult = Invoke-RestMethod -Method Post -Uri "$baseUri/api/qr/check-in" `
        -Headers $scannerHeaders -ContentType "application/json" -Body $checkIn
    Assert-FieldEquals $checkInResult "accepted" $true "QR check-in"
    Assert-FieldIn $checkInResult "status" @("started", "extended", "already_active") "QR check-in"
    Assert-FieldEquals $checkInResult "objectId" $workerObjectId "QR check-in"
    Assert-NonEmptyField $checkInResult "bindingId" "QR check-in"

    Write-Host "[5/10] Bound worker enters forbidden zone -> violation"
    $enterEventId = "smoke-worker-enter-$([guid]::NewGuid().ToString('N'))"
    $enterEvent = @{
        eventId = $enterEventId
        eventName = $eventName
        sourceName = $sourceName
        receivedAt = Get-UtcTimestamp
        cameraId = $cameraId
        objectId = $workerObjectId
        action = "enter"
    } | ConvertTo-Json -Compress
    $enterDecision = Invoke-RestMethod -Method Post `
        -Uri "$baseUri/api/xprotect/line-crossing" -Headers $headers `
        -ContentType "application/json" -Body $enterEvent
    Assert-FieldEquals $enterDecision "accepted" $true "Worker LineCrossing"
    Assert-FieldEquals $enterDecision "decision" "violation" "Worker LineCrossing"
    Assert-FieldEquals $enterDecision "zoneId" "SMOKE-ZONE" "Worker LineCrossing"
    Assert-FieldEquals $enterDecision "employeeId" "SMOKE-EMPLOYEE" "Worker LineCrossing"

    Write-Host "[6/10] Verify active MongoDB alarm and manager notification"
    Invoke-MongoScript -Script $activeAlarmAssertion | Out-Null

    Write-Host "[7/10] Replay same event -> duplicate without a second alarm"
    $duplicateDecision = Invoke-RestMethod -Method Post `
        -Uri "$baseUri/api/xprotect/line-crossing" -Headers $headers `
        -ContentType "application/json" -Body $enterEvent
    Assert-FieldEquals $duplicateDecision "duplicate" $true "Duplicate LineCrossing"
    Invoke-MongoScript -Script $activeAlarmAssertion | Out-Null

    Write-Host "[8/10] Worker exits forbidden zone -> alarm cleared"
    $exitEvent = @{
        eventId = "smoke-worker-exit-$([guid]::NewGuid().ToString('N'))"
        eventName = $eventName
        sourceName = $sourceName
        receivedAt = Get-UtcTimestamp
        cameraId = $cameraId
        objectId = $workerObjectId
        action = "exit"
    } | ConvertTo-Json -Compress
    $exitDecision = Invoke-RestMethod -Method Post `
        -Uri "$baseUri/api/xprotect/line-crossing" -Headers $headers `
        -ContentType "application/json" -Body $exitEvent
    Assert-FieldEquals $exitDecision "accepted" $true "Worker exit"
    Assert-FieldEquals $exitDecision "decision" "cleared" "Worker exit"

    Write-Host "[9/10] Verify resolved alarm and manager clear notification"
    Invoke-MongoScript -Script $resolvedAlarmAssertion | Out-Null

    $flowSucceeded = $true
}
finally {
    Write-Host "[10/10] Cleanup"
    $cleanupSucceeded = Invoke-MongoScript -Script $cleanupScript -IgnoreErrors
}

if ($flowSucceeded -and -not $cleanupSucceeded) {
    throw "Smoke flow passed, but isolated fixture cleanup failed."
}

Write-Host "SecureZone full local event injection flow passed."
