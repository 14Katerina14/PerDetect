param(
    [string]$BackupRoot = "C:\ProgramData\SecureZone\XProtectPlugin\backups"
)

$ErrorActionPreference = "Stop"

if (-not ([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole(
    [Security.Principal.WindowsBuiltInRole]::Administrator
)) {
    throw "Run rollback from PowerShell as Administrator."
}

$backup = Get-ChildItem -LiteralPath $BackupRoot -Directory -ErrorAction Stop |
    Sort-Object Name -Descending |
    Select-Object -First 1
if (-not $backup) { throw "No SecureZone plug-in backup was found in $BackupRoot" }

$statePath = Join-Path $backup.FullName "rollback-state.json"
if (-not (Test-Path -LiteralPath $statePath -PathType Leaf)) {
    throw "Backup is missing rollback-state.json: $($backup.FullName)"
}
$state = Get-Content -LiteralPath $statePath -Raw | ConvertFrom-Json

$service = Get-Service -DisplayName "Milestone XProtect Event Server" -ErrorAction Stop
$wasRunning = $service.Status -eq "Running"
if ($wasRunning) {
    Stop-Service -InputObject $service -Force
    $service.WaitForStatus("Stopped", [TimeSpan]::FromSeconds(30))
}

try {
    foreach ($fileName in @("SecureZone.XProtectPlugin.dll", "plugin.def")) {
        $destination = Join-Path $state.pluginFolder $fileName
        if ($state.previousFiles -contains $fileName) {
            Copy-Item -LiteralPath (Join-Path $backup.FullName $fileName) -Destination $destination -Force
        } elseif (Test-Path -LiteralPath $destination) {
            Remove-Item -LiteralPath $destination -Force
        }
    }
    foreach ($property in $state.previousEnvironment.PSObject.Properties) {
        [Environment]::SetEnvironmentVariable($property.Name, $property.Value, "Machine")
    }
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

Write-Host "Restored SecureZone plug-in state from $($backup.FullName)"
