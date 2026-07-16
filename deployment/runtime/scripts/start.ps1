$ErrorActionPreference = "Stop"

$runtimeDirectory = Split-Path -Parent $PSScriptRoot
$composeFile = Join-Path $runtimeDirectory "docker-compose.yml"
$envFile = Join-Path $runtimeDirectory ".env"

if (-not (Test-Path -LiteralPath $envFile -PathType Leaf)) {
    throw "Missing deployment/runtime/.env. Copy .env.example to .env and replace every placeholder."
}

Write-Host "Validating SecureZone runtime configuration..."
docker compose --env-file $envFile -f $composeFile config --quiet
if ($LASTEXITCODE -ne 0) { throw "Docker Compose configuration validation failed." }

Write-Host "Building and starting SecureZone runtime..."
docker compose --env-file $envFile -f $composeFile up -d --build
if ($LASTEXITCODE -ne 0) { throw "Docker Compose failed to start the SecureZone runtime." }

docker compose --env-file $envFile -f $composeFile ps
if ($LASTEXITCODE -ne 0) { throw "Docker Compose could not report service status." }
