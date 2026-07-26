$ErrorActionPreference = "Stop"

$runtimeDirectory = Split-Path -Parent $PSScriptRoot
$composeFile = Join-Path $runtimeDirectory "docker-compose.yml"
$envFile = Join-Path $runtimeDirectory ".env"

if (-not (Test-Path -LiteralPath $envFile -PathType Leaf)) {
    throw "Missing deployment/runtime/.env. The same environment file used to start the deployment is required."
}

docker compose --env-file $envFile -f $composeFile down
if ($LASTEXITCODE -ne 0) { throw "Docker Compose failed to stop the SecureZone runtime." }
