#!/usr/bin/env bash
set -euo pipefail

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
runtime_dir="$(cd -- "${script_dir}/.." && pwd)"
compose_file="${runtime_dir}/docker-compose.yml"
env_file="${runtime_dir}/.env"

if [[ ! -f "${env_file}" ]]; then
  echo "Error: missing deployment/runtime/.env. Copy .env.example to .env and replace every placeholder." >&2
  exit 1
fi

echo "Validating SecureZone runtime configuration..."
docker compose --env-file "${env_file}" -f "${compose_file}" config --quiet

echo "Building and starting SecureZone runtime..."
docker compose --env-file "${env_file}" -f "${compose_file}" up -d --build --wait --wait-timeout 180
docker compose --env-file "${env_file}" -f "${compose_file}" ps
