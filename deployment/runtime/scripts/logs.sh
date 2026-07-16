#!/usr/bin/env bash
set -euo pipefail

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
runtime_dir="$(cd -- "${script_dir}/.." && pwd)"
compose_file="${runtime_dir}/docker-compose.yml"
env_file="${runtime_dir}/.env"
service="${1:-}"

if [[ ! -f "${env_file}" ]]; then
  echo "Error: missing deployment/runtime/.env. The environment file used to start the deployment is required." >&2
  exit 1
fi

if [[ -n "${service}" && "${service}" != "mongodb" && "${service}" != "securezone-api" ]]; then
  echo "Error: unknown service '${service}'. Use 'mongodb' or 'securezone-api'." >&2
  exit 1
fi

args=(compose --env-file "${env_file}" -f "${compose_file}" logs --tail 200 -f)
if [[ -n "${service}" ]]; then
  args+=("${service}")
fi
docker "${args[@]}"
