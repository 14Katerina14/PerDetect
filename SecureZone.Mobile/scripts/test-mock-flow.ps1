param([string]$BaseUrl = "http://127.0.0.1:19080")

$ErrorActionPreference = "Stop"
$health = Invoke-RestMethod -Uri "$BaseUrl/health"
if ($health.status -ne "ok") { throw "Health check failed." }

$loginBody = @{ username = "manager"; password = "demo" } | ConvertTo-Json
$login = Invoke-RestMethod -Method Post -Uri "$BaseUrl/api/auth/login" -ContentType "application/json" -Body $loginBody
$headers = @{ Authorization = "Bearer $($login.accessToken)" }

$scannerBody = @{ username = "scanner"; password = "demo" } | ConvertTo-Json
$scanner = Invoke-RestMethod -Method Post -Uri "$BaseUrl/api/auth/login" -ContentType "application/json" -Body $scannerBody
$scannerHeaders = @{ Authorization = "Bearer $($scanner.accessToken)" }
$requestId = "home-test-$([DateTimeOffset]::UtcNow.ToUnixTimeMilliseconds())"
$checkInBody = @{
    employeeId = "EMP-001"
    zoneId = "ZONE-001"
    cameraId = "CAM-001"
    requestId = $requestId
} | ConvertTo-Json
$firstCheckIn = Invoke-RestMethod -Method Post -Uri "$BaseUrl/api/qr/check-in" -Headers $scannerHeaders -ContentType "application/json" -Body $checkInBody
$repeatedCheckIn = Invoke-RestMethod -Method Post -Uri "$BaseUrl/api/qr/check-in" -Headers $scannerHeaders -ContentType "application/json" -Body $checkInBody
if (-not $firstCheckIn.accepted -or $firstCheckIn.sessionId -ne $repeatedCheckIn.sessionId) { throw "QR idempotency flow failed." }

Invoke-RestMethod -Method Post -Uri "$BaseUrl/test/alarm/raise" | Out-Null
$active = Invoke-RestMethod -Uri "$BaseUrl/api/alarms/active" -Headers $headers
if ($active.count -ne 1 -or -not $active.alarms[0].stillInside) { throw "Alarm raise flow failed." }

Invoke-RestMethod -Method Post -Uri "$BaseUrl/test/alarm/clear" | Out-Null
$recent = Invoke-RestMethod -Uri "$BaseUrl/api/alarms/recent" -Headers $headers
if ($recent.count -ne 1 -or $recent.alarms[0].stillInside) { throw "Alarm clear flow failed." }

Write-Host "SecureZone local mobile flow passed: health, role login, idempotent QR, alarm raise and alarm clear."
