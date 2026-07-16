#!/usr/bin/env bash
set -euo pipefail

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
runtime_dir="$(cd -- "${script_dir}/.." && pwd)"
env_file="${runtime_dir}/.env"

if [[ ! -f "${env_file}" ]]; then
  echo "Error: missing deployment/runtime/.env. Copy .env.example to .env and replace every placeholder." >&2
  exit 1
fi

read_env_value() {
  local key="$1"
  awk -v wanted="${key}" '
    /^[[:space:]]*(#|$)/ { next }
    {
      line = $0
      sub(/\r$/, "", line)
      separator = index(line, "=")
      if (separator == 0) next
      name = substr(line, 1, separator - 1)
      gsub(/^[[:space:]]+|[[:space:]]+$/, "", name)
      if (name != wanted) next
      value = substr(line, separator + 1)
      gsub(/^[[:space:]]+|[[:space:]]+$/, "", value)
      if (length(value) >= 2 && ((substr(value, 1, 1) == "\"" && substr(value, length(value)) == "\"") || (substr(value, 1, 1) == "\047" && substr(value, length(value)) == "\047"))) {
        value = substr(value, 2, length(value) - 2)
      }
      print value
      exit
    }
  ' "${env_file}"
}

json_has_string() {
  local json="$1" field="$2" expected="$3"
  grep -Eq "\"${field}\"[[:space:]]*:[[:space:]]*\"${expected}\"" <<<"${json}"
}

json_has_true() {
  local json="$1" field="$2"
  grep -Eq "\"${field}\"[[:space:]]*:[[:space:]]*true" <<<"${json}"
}

api_port="$(read_env_value SECUREZONE_API_HOST_PORT)"
api_port="${api_port:-8080}"
api_key="$(read_env_value SECUREZONE_XPROTECT_API_KEY)"
if [[ -z "${api_key}" || "${api_key}" == \<* ]]; then
  echo "Error: SECUREZONE_XPROTECT_API_KEY must have a real local development value in deployment/runtime/.env." >&2
  exit 1
fi

base_uri="http://127.0.0.1:${api_port}"
event_name='Channel.<int>.OpenSDK.WiseAI.LineCrossing.<int>.State-2'
source_name='Hanwha Vision TNO-C4052T TEST-CAMERA - Camera 1'

echo "[1/4] Health check"
health="$(curl --fail-with-body --silent --show-error "${base_uri}/health")"
json_has_string "${health}" status ok || { echo "Error: health check returned an unexpected JSON status." >&2; exit 1; }

echo "[2/4] Line crossing without QR presence -> violation"
first_id="smoke-before-$(date -u +%s%N)-${RANDOM}"
received_at="$(date -u +%Y-%m-%dT%H:%M:%SZ)"
first_payload="$(printf '{\"eventId\":\"%s\",\"eventName\":\"%s\",\"sourceName\":\"%s\",\"receivedAt\":\"%s\"}' "${first_id}" "${event_name}" "${source_name}" "${received_at}")"
first_result="$(curl --fail-with-body --silent --show-error --request POST \
  --header 'Content-Type: application/json' \
  --header "X-SecureZone-Api-Key: ${api_key}" \
  --data "${first_payload}" \
  "${base_uri}/api/xprotect/line-crossing")"
json_has_true "${first_result}" accepted || { echo "Error: initial line crossing was not accepted." >&2; exit 1; }
json_has_string "${first_result}" decision violation || { echo "Error: initial line crossing did not return violation." >&2; exit 1; }

echo "[3/4] QR check-in -> accepted"
check_in_payload='{"employeeId":"EMP-001","zoneId":"ZONE-001","scannedByUserId":"APP-SCANNER-001"}'
check_in_result="$(curl --fail-with-body --silent --show-error --request POST \
  --header 'Content-Type: application/json' \
  --data "${check_in_payload}" \
  "${base_uri}/api/qr/check-in")"
json_has_true "${check_in_result}" accepted || { echo "Error: QR check-in was not accepted." >&2; exit 1; }

echo "[4/4] Line crossing with active QR presence -> allowed"
second_id="smoke-after-$(date -u +%s%N)-${RANDOM}"
received_at="$(date -u +%Y-%m-%dT%H:%M:%SZ)"
second_payload="$(printf '{\"eventId\":\"%s\",\"eventName\":\"%s\",\"sourceName\":\"%s\",\"receivedAt\":\"%s\"}' "${second_id}" "${event_name}" "${source_name}" "${received_at}")"
second_result="$(curl --fail-with-body --silent --show-error --request POST \
  --header 'Content-Type: application/json' \
  --header "X-SecureZone-Api-Key: ${api_key}" \
  --data "${second_payload}" \
  "${base_uri}/api/xprotect/line-crossing")"
json_has_true "${second_result}" accepted || { echo "Error: post-check-in line crossing was not accepted." >&2; exit 1; }
json_has_string "${second_result}" decision allowed || { echo "Error: post-check-in line crossing did not return allowed." >&2; exit 1; }

echo "SecureZone runtime smoke test passed."
