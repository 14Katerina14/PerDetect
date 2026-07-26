param(
    [Parameter(Mandatory)]
    [string]$BackendHost,
    [int]$BackendPort = 18080,
    [string]$ApiKeyFile = (Join-Path $PSScriptRoot "securezone-api-key.txt"),
    [string]$PluginFolder = "C:\Program Files\Milestone\MIPPlugins\SecureZone.XProtectPlugin"
)

$ErrorActionPreference = "Stop"

if (-not ([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole(
    [Security.Principal.WindowsBuiltInRole]::Administrator
)) {
    throw "Run this installer from PowerShell as Administrator."
}

if (-not (Test-Path -LiteralPath $ApiKeyFile -PathType Leaf)) {
    throw "Missing API key file: $ApiKeyFile"
}

$apiKey = (Get-Content -LiteralPath $ApiKeyFile -Raw).Trim()
if ($apiKey.Length -lt 32) {
    throw "The API key file must contain at least 32 characters."
}

$baseUrl = "http://${BackendHost}:$BackendPort"
$endpoint = "$baseUrl/api/xprotect/line-crossing"
Write-Host "Checking SecureZone backend at $baseUrl ..."
$health = Invoke-RestMethod -Method Get -Uri "$baseUrl/health" -TimeoutSec 10
if ($health.status -ne "ok") {
    throw "SecureZone backend health check failed."
}

$service = Get-Service -DisplayName "Milestone XProtect Event Server" -ErrorAction Stop
$wasRunning = $service.Status -eq "Running"
if ($wasRunning) {
    Stop-Service -InputObject $service -Force
    $service.WaitForStatus("Stopped", [TimeSpan]::FromSeconds(30))
}

try {
    & (Join-Path $PSScriptRoot "install-on-xprotect-server.ps1") `
        -PluginFolder $PluginFolder `
        -ApiUrl $endpoint `
        -ApiKey $apiKey
}
finally {
    if ($wasRunning) {
        Start-Service -DisplayName "Milestone XProtect Event Server"
        (Get-Service -DisplayName "Milestone XProtect Event Server").WaitForStatus(
            "Running",
            [TimeSpan]::FromSeconds(30)
        )
    }
}

Write-Host "SecureZone plug-in installation completed."
Write-Host "Run .\preflight-office.ps1 -BackendHost $BackendHost to verify it."
