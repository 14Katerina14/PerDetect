param(
    [Parameter(Position = 0)]
    [string]$Service
)

$ErrorActionPreference = "Stop"

$runtimeDirectory = Split-Path -Parent $PSScriptRoot
$composeFile = Join-Path $runtimeDirectory "docker-compose.yml"
$envFile = Join-Path $runtimeDirectory ".env"

if (-not (Test-Path -LiteralPath $envFile -PathType Leaf)) {
    throw "Missing deployment/runtime/.env. The same environment file used to start the deployment is required."
}

if ($Service -and $Service -notin @("mongodb", "securezone-api")) {
    throw "Unknown service '$Service'. Use 'mongodb' or 'securezone-api'."
}

$arguments = @("compose", "--env-file", $envFile, "-f", $composeFile, "logs", "--tail", "200", "-f")
if ($Service) { $arguments += $Service }

& docker @arguments
if ($LASTEXITCODE -ne 0) { throw "Docker Compose logs command failed." }
