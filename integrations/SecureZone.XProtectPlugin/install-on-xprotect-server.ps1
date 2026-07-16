param(
    [string]$PluginFolder = "C:\Program Files\Milestone\MIPPlugins\SecureZone.XProtectPlugin",
    [string]$ApiUrl = "http://127.0.0.1:8080/api/xprotect/line-crossing",
    [string]$ApiKey = "",
    [ValidateRange(1, 60)]
    [int]$TimeoutSeconds = 5,
    [ValidateRange(1, 60)]
    [int]$MetadataHeartbeatSeconds = 5,
    [ValidateRange(2, 120)]
    [int]$MetadataLostAfterSeconds = 8
)

$ErrorActionPreference = "Stop"

$sourceFolder = Split-Path -Parent $MyInvocation.MyCommand.Path
$dllPath = Join-Path $sourceFolder "SecureZone.XProtectPlugin.dll"
$pluginDefPath = Join-Path $sourceFolder "plugin.def"

if (-not (Test-Path -LiteralPath $dllPath)) {
    throw "Missing SecureZone.XProtectPlugin.dll in $sourceFolder"
}

if (-not (Test-Path -LiteralPath $pluginDefPath)) {
    throw "Missing plugin.def in $sourceFolder"
}

$apiUri = $null
if (-not [Uri]::TryCreate($ApiUrl, [UriKind]::Absolute, [ref]$apiUri) -or
    $apiUri.Scheme -notin @("http", "https")) {
    throw "ApiUrl must be an absolute HTTP or HTTPS URL."
}

New-Item -ItemType Directory -Path $PluginFolder -Force | Out-Null
Copy-Item -LiteralPath $dllPath -Destination $PluginFolder -Force
Copy-Item -LiteralPath $pluginDefPath -Destination $PluginFolder -Force

Unblock-File -LiteralPath (Join-Path $PluginFolder "SecureZone.XProtectPlugin.dll") -ErrorAction SilentlyContinue
Unblock-File -LiteralPath (Join-Path $PluginFolder "plugin.def") -ErrorAction SilentlyContinue

[Environment]::SetEnvironmentVariable("SECUREZONE_API_URL", $ApiUrl, "Machine")
[Environment]::SetEnvironmentVariable("SECUREZONE_API_TIMEOUT_SECONDS", $TimeoutSeconds.ToString(), "Machine")
[Environment]::SetEnvironmentVariable("SECUREZONE_METADATA_HEARTBEAT_SECONDS", $MetadataHeartbeatSeconds.ToString(), "Machine")
[Environment]::SetEnvironmentVariable("SECUREZONE_METADATA_LOST_AFTER_SECONDS", $MetadataLostAfterSeconds.ToString(), "Machine")

if (-not [string]::IsNullOrWhiteSpace($ApiKey)) {
    [Environment]::SetEnvironmentVariable("SECUREZONE_XPROTECT_API_KEY", $ApiKey, "Machine")
}

Write-Host "SecureZone XProtect decision bridge installed to:"
Write-Host "  $PluginFolder"
Write-Host "Backend endpoint:"
Write-Host "  $ApiUrl"

if ([string]::IsNullOrWhiteSpace($ApiKey)) {
    Write-Warning "No API key was supplied. Configure the same SECUREZONE_XPROTECT_API_KEY on the plugin and backend before production use."
}

Write-Host ""
Write-Host "Restart the Milestone XProtect Event Server service to load the new plugin and machine environment variables."
Write-Host '  Restart-Service -DisplayName "Milestone XProtect Event Server" -Force'
