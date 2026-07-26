param(
    [Parameter(Mandatory)]
    [string]$BackendHost,
    [int]$BackendPort = 18080,
    [string]$ApiKeyFile = (Join-Path $PSScriptRoot "securezone-api-key.txt"),
    [string]$PluginFolder = "C:\Program Files\Milestone\MIPPlugins\SecureZone.XProtectPlugin",
    [string]$ReportPath = (Join-Path $PSScriptRoot ("preflight-{0}.txt" -f [DateTime]::Now.ToString("yyyyMMdd-HHmmss")))
)

$ErrorActionPreference = "Stop"
$results = [System.Collections.Generic.List[string]]::new()
$failed = $false

function Add-Check {
    param([string]$Name, [bool]$Passed, [string]$Details)
    $script:failed = $script:failed -or -not $Passed
    $line = "[{0}] {1}: {2}" -f $(if ($Passed) { "PASS" } else { "FAIL" }), $Name, $Details
    $results.Add($line)
    Write-Host $line
}

$baseUrl = "http://${BackendHost}:$BackendPort"
$endpoint = "$baseUrl/api/xprotect/line-crossing"
$apiKey = if (Test-Path -LiteralPath $ApiKeyFile) { (Get-Content -LiteralPath $ApiKeyFile -Raw).Trim() } else { "" }

try {
    $health = Invoke-RestMethod -Method Get -Uri "$baseUrl/health" -TimeoutSec 10
    Add-Check "Backend health" ($health.status -eq "ok") "$baseUrl/health"
} catch { Add-Check "Backend health" $false $_.Exception.Message }

try {
    $version = Invoke-RestMethod -Method Get -Uri "$baseUrl/version" -TimeoutSec 10
    Add-Check "Backend version" (-not [string]::IsNullOrWhiteSpace([string]$version.buildId)) ("buildId=" + $version.buildId)
} catch { Add-Check "Backend version" $false $_.Exception.Message }

Add-Check "API key file" ($apiKey.Length -ge 32) $(if ($apiKey) { "present; value hidden" } else { "missing" })

if ($apiKey.Length -ge 32) {
    try {
        Invoke-WebRequest -Method Post -Uri $endpoint -Headers @{ "X-SecureZone-Api-Key" = $apiKey } `
            -ContentType "application/json" -Body "{}" -TimeoutSec 10 -UseBasicParsing | Out-Null
        Add-Check "Protected XProtect route" $false "Unexpected success for invalid request body"
    } catch {
        $status = [int]$_.Exception.Response.StatusCode
        Add-Check "Protected XProtect route" ($status -eq 400) "HTTP $status (expected 400 for authenticated invalid body)"
    }
}

$dllPath = Join-Path $PluginFolder "SecureZone.XProtectPlugin.dll"
$defPath = Join-Path $PluginFolder "plugin.def"
Add-Check "Installed plug-in DLL" (Test-Path -LiteralPath $dllPath -PathType Leaf) $dllPath
Add-Check "Installed plugin.def" (Test-Path -LiteralPath $defPath -PathType Leaf) $defPath
if (Test-Path -LiteralPath $dllPath -PathType Leaf) {
    $version = [Diagnostics.FileVersionInfo]::GetVersionInfo($dllPath).FileVersion
    Add-Check "Plug-in version" ($version -eq "2.0.0.0") "version=$version"
}

$configuredUrl = [Environment]::GetEnvironmentVariable("SECUREZONE_API_URL", "Machine")
$configuredKey = [Environment]::GetEnvironmentVariable("SECUREZONE_XPROTECT_API_KEY", "Machine")
Add-Check "Plug-in backend URL" ($configuredUrl -eq $endpoint) "configured=$configuredUrl"
Add-Check "Plug-in API key" ($apiKey.Length -ge 32 -and $configuredKey -eq $apiKey) "matches backend key; value hidden"

try {
    $service = Get-Service -DisplayName "Milestone XProtect Event Server" -ErrorAction Stop
    Add-Check "XProtect Event Server" ($service.Status -eq "Running") ("status=" + $service.Status)
} catch { Add-Check "XProtect Event Server" $false $_.Exception.Message }

$header = @(
    "SecureZone office preflight",
    "Generated: $([DateTime]::Now.ToString('o'))",
    "Backend: $baseUrl",
    "API key: hidden",
    ""
)
New-Item -ItemType Directory -Path (Split-Path -Parent $ReportPath) -Force | Out-Null
($header + $results) | Set-Content -LiteralPath $ReportPath -Encoding UTF8
Write-Host "Report: $ReportPath"

if ($failed) { exit 1 }
