param(
    [string]$PluginFolder = "C:\Program Files\Milestone\MIPPlugins\SecureZone.XProtectPlugin"
)

$ErrorActionPreference = "Stop"

$sourceFolder = Split-Path -Parent $MyInvocation.MyCommand.Path
$dllPath = Join-Path $sourceFolder "SecureZone.XProtectPlugin.dll"
$pluginDefPath = Join-Path $sourceFolder "plugin.def"

if (-not (Test-Path $dllPath)) {
    throw "Missing SecureZone.XProtectPlugin.dll in $sourceFolder"
}

if (-not (Test-Path $pluginDefPath)) {
    throw "Missing plugin.def in $sourceFolder"
}

New-Item -ItemType Directory -Path $PluginFolder -Force | Out-Null

Copy-Item -Path $dllPath -Destination $PluginFolder -Force
Copy-Item -Path $pluginDefPath -Destination $PluginFolder -Force

Unblock-File -Path (Join-Path $PluginFolder "SecureZone.XProtectPlugin.dll") -ErrorAction SilentlyContinue
Unblock-File -Path (Join-Path $PluginFolder "plugin.def") -ErrorAction SilentlyContinue

Write-Host "SecureZone XProtect plugin installed to:"
Write-Host "  $PluginFolder"
Write-Host ""
Write-Host "Restart the XProtect Event Server service next."
Write-Host "If the service display name is the default, run:"
Write-Host '  Restart-Service -DisplayName "Milestone XProtect Event Server" -Force'
