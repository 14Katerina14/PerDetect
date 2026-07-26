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

json_has_any_string() {
  local json="$1" field="$2" expected_pattern="$3"
  grep -Eq "\"${field}\"[[:space:]]*:[[:space:]]*\"(${expected_pattern})\"" <<<"${json}"
}

json_has_nonempty_string() {
  local json="$1" field="$2"
  grep -Eq "\"${field}\"[[:space:]]*:[[:space:]]*\"[^\"]+\"" <<<"${json}"
}

json_has_true() {
  local json="$1" field="$2"
  grep -Eq "\"${field}\"[[:space:]]*:[[:space:]]*true" <<<"${json}"
}

mongo_exec() {
  local mongo_script="$1"
  printf '%s\n' "${mongo_script}" | docker compose \
    --env-file "${env_file}" -f "${compose_file}" \
    exec -T mongodb sh -lc \
    'exec mongosh --quiet --username "$MONGO_INITDB_ROOT_USERNAME" --password "$MONGO_INITDB_ROOT_PASSWORD" --authenticationDatabase admin "$MONGO_INITDB_DATABASE" --file /dev/stdin'
}

cleanup_script='const database = db.getSiblingDB(process.env.MONGO_INITDB_DATABASE || "securezone");
database.qr_checkins.deleteMany({$or:[{employeeId:"SMOKE-EMPLOYEE"},{zoneId:"SMOKE-ZONE"},{scannedByUserId:"SMOKE-SCANNER"}]});
database.presence_sessions.deleteMany({$or:[{employeeId:"SMOKE-EMPLOYEE"},{zoneId:"SMOKE-ZONE"}]});
database.camera_object_tracks.deleteMany({cameraId:"CAM-SMOKE"});
database.track_identity_bindings.deleteMany({$or:[{cameraId:"CAM-SMOKE"},{employeeId:"SMOKE-EMPLOYEE"}]});
database.alarms.deleteMany({$or:[{zoneId:"SMOKE-ZONE"},{employeeId:"SMOKE-EMPLOYEE"},{machineId:"SMOKE-MACHINE"},{trackId:{$regex:"^CAM-SMOKE:"}}]});
database.access_policies.deleteMany({policyId:"SMOKE-POLICY"});
database.zones.deleteMany({zoneId:"SMOKE-ZONE"});
database.machines.deleteMany({machineId:"SMOKE-MACHINE"});
database.app_users.deleteMany({userId:"SMOKE-SCANNER"});
database.employees.deleteMany({employeeId:"SMOKE-EMPLOYEE"});'

cleanup() {
  echo "Cleaning isolated smoke fixtures..."
  if ! mongo_exec "${cleanup_script}" >/dev/null; then
    echo "Warning: smoke fixture cleanup failed; inspect the MongoDB container before rerunning." >&2
    return 1
  fi
}

setup_script="${cleanup_script}
database.employees.insertOne({employeeId:\"SMOKE-EMPLOYEE\",fullName:\"Smoke Test Employee\",department:\"Testing\",roles:[\"maintenance\"],status:\"active\",qrTokenHash:\"smoke-test-token-hash\"});
database.machines.insertOne({machineId:\"SMOKE-MACHINE\",name:\"Smoke Test Machine\",status:\"stopped\",updatedAt:new Date()});
database.zones.insertOne({zoneId:\"SMOKE-ZONE\",name:\"Smoke Test Dangerous Zone\",cameraId:\"CAM-SMOKE\",type:\"dangerous\",status:\"active\",relatedMachineId:\"SMOKE-MACHINE\",xprotectEventName:\"SecureZone.OpenSDK.WiseAI.LineCrossing.Smoke.State-2\"});
database.access_policies.insertOne({policyId:\"SMOKE-POLICY\",zoneId:\"SMOKE-ZONE\",allowedRoles:[\"maintenance\"],machineStatesAllowed:[\"stopped\",\"maintenance\"],timeWindows:[]});"

api_port="$(read_env_value SECUREZONE_API_HOST_PORT)"
api_port="${api_port:-18080}"
api_key="$(read_env_value SECUREZONE_XPROTECT_API_KEY)"
if [[ -z "${api_key}" || "${api_key}" == \<* ]]; then
  echo "Error: SECUREZONE_XPROTECT_API_KEY must have a real local development value in deployment/runtime/.env." >&2
  exit 1
fi

base_uri="http://127.0.0.1:${api_port}"
event_name='SecureZone.OpenSDK.WiseAI.LineCrossing.Smoke.State-2'
source_name='SecureZone Smoke Camera'
camera_id='CAM-SMOKE'
unknown_object_id='SMOKE-UNKNOWN-OBJECT'
authorized_object_id='SMOKE-AUTHORIZED-OBJECT'

echo "[1/8] Health check"
health="$(curl --fail-with-body --silent --show-error "${base_uri}/health")"
json_has_string "${health}" status ok || { echo "Error: health check returned an unexpected JSON status." >&2; exit 1; }

echo "Preparing isolated MongoDB smoke fixtures..."
trap cleanup EXIT
mongo_exec "${setup_script}" >/dev/null

echo "[2/8] Provision scanner and obtain JWT"
scanner_password="Smoke-$(date -u +%s%N)-${RANDOM}${RANDOM}"
printf '%s\n' "${scanner_password}" | docker compose \
  --env-file "${env_file}" -f "${compose_file}" \
  exec -T securezone-api /app/SecureZone.ProvisionUser \
  --user-id SMOKE-SCANNER --username smoke-scanner --role scanner >/dev/null
login_payload="$(printf '{\"username\":\"smoke-scanner\",\"password\":\"%s\"}' "${scanner_password}")"
login_result="$(curl --fail-with-body --silent --show-error --request POST \
  --header 'Content-Type: application/json' \
  --data "${login_payload}" \
  "${base_uri}/api/auth/login")"
access_token="$(sed -n 's/.*\"accessToken\"[[:space:]]*:[[:space:]]*\"\([^\"]*\)\".*/\1/p' <<<"${login_result}")"
unset scanner_password
[[ -n "${access_token}" ]] || { echo "Error: scanner login did not return an access token." >&2; exit 1; }

echo "[3/8] Unknown camera object -> violation"
unknown_enter_id="smoke-unknown-enter-$(date -u +%s%N)-${RANDOM}"
received_at="$(date -u +%Y-%m-%dT%H:%M:%S.%NZ)"
unknown_enter_payload="$(printf '{\"eventId\":\"%s\",\"eventName\":\"%s\",\"sourceName\":\"%s\",\"receivedAt\":\"%s\",\"cameraId\":\"%s\",\"objectId\":\"%s\",\"action\":\"enter\"}' "${unknown_enter_id}" "${event_name}" "${source_name}" "${received_at}" "${camera_id}" "${unknown_object_id}")"
unknown_enter_result="$(curl --fail-with-body --silent --show-error --request POST \
  --header 'Content-Type: application/json' \
  --header "X-SecureZone-Api-Key: ${api_key}" \
  --data "${unknown_enter_payload}" \
  "${base_uri}/api/xprotect/line-crossing")"
json_has_true "${unknown_enter_result}" accepted || { echo "Error: unknown-object LineCrossing was not accepted." >&2; exit 1; }
json_has_string "${unknown_enter_result}" decision violation || { echo "Error: unknown object did not return violation." >&2; exit 1; }

echo "[4/8] Unknown camera object exit -> alarm cleared"
unknown_exit_id="smoke-unknown-exit-$(date -u +%s%N)-${RANDOM}"
received_at="$(date -u +%Y-%m-%dT%H:%M:%S.%NZ)"
unknown_exit_payload="$(printf '{\"eventId\":\"%s\",\"eventName\":\"%s\",\"sourceName\":\"%s\",\"receivedAt\":\"%s\",\"cameraId\":\"%s\",\"objectId\":\"%s\",\"action\":\"exit\"}' "${unknown_exit_id}" "${event_name}" "${source_name}" "${received_at}" "${camera_id}" "${unknown_object_id}")"
unknown_exit_result="$(curl --fail-with-body --silent --show-error --request POST \
  --header 'Content-Type: application/json' \
  --header "X-SecureZone-Api-Key: ${api_key}" \
  --data "${unknown_exit_payload}" \
  "${base_uri}/api/xprotect/line-crossing")"
json_has_true "${unknown_exit_result}" accepted || { echo "Error: unknown-object exit was not accepted." >&2; exit 1; }
json_has_string "${unknown_exit_result}" decision cleared || { echo "Error: unknown-object exit did not clear the alarm." >&2; exit 1; }

echo "[5/8] Recent Human camera observation"
observed_at="$(date -u +%Y-%m-%dT%H:%M:%S.%NZ)"
observation_payload="$(printf '{\"cameraId\":\"%s\",\"objectId\":\"%s\",\"objectType\":\"Human\",\"observedAt\":\"%s\"}' "${camera_id}" "${authorized_object_id}" "${observed_at}")"
observation_result="$(curl --fail-with-body --silent --show-error --request POST \
  --header 'Content-Type: application/json' \
  --header "X-SecureZone-Api-Key: ${api_key}" \
  --data "${observation_payload}" \
  "${base_uri}/api/xprotect/object-observations")"
json_has_true "${observation_result}" accepted || { echo "Error: Human observation was not accepted." >&2; exit 1; }
json_has_string "${observation_result}" status observed || { echo "Error: Human observation did not return status observed." >&2; exit 1; }

echo "[6/8] Authenticated QR check-in -> camera object identity binding"
check_in_payload="$(printf '{\"employeeId\":\"SMOKE-EMPLOYEE\",\"zoneId\":\"SMOKE-ZONE\",\"cameraId\":\"%s\"}' "${camera_id}")"
check_in_result="$(curl --fail-with-body --silent --show-error --request POST \
  --header 'Content-Type: application/json' \
  --header "Authorization: Bearer ${access_token}" \
  --data "${check_in_payload}" \
  "${base_uri}/api/qr/check-in")"
json_has_true "${check_in_result}" accepted || { echo "Error: QR check-in was not accepted." >&2; exit 1; }
json_has_any_string "${check_in_result}" status 'started|extended|already_active' || { echo "Error: QR check-in returned an unexpected status." >&2; exit 1; }
json_has_string "${check_in_result}" objectId "${authorized_object_id}" || { echo "Error: QR check-in bound the wrong camera object." >&2; exit 1; }
json_has_nonempty_string "${check_in_result}" bindingId || { echo "Error: QR check-in did not return a bindingId." >&2; exit 1; }

echo "[7/8] Bound camera object -> allowed"
authorized_event_id="smoke-authorized-enter-$(date -u +%s%N)-${RANDOM}"
received_at="$(date -u +%Y-%m-%dT%H:%M:%S.%NZ)"
authorized_payload="$(printf '{\"eventId\":\"%s\",\"eventName\":\"%s\",\"sourceName\":\"%s\",\"receivedAt\":\"%s\",\"cameraId\":\"%s\",\"objectId\":\"%s\",\"action\":\"enter\"}' "${authorized_event_id}" "${event_name}" "${source_name}" "${received_at}" "${camera_id}" "${authorized_object_id}")"
authorized_result="$(curl --fail-with-body --silent --show-error --request POST \
  --header 'Content-Type: application/json' \
  --header "X-SecureZone-Api-Key: ${api_key}" \
  --data "${authorized_payload}" \
  "${base_uri}/api/xprotect/line-crossing")"
json_has_true "${authorized_result}" accepted || { echo "Error: authorized-object LineCrossing was not accepted." >&2; exit 1; }
json_has_string "${authorized_result}" decision allowed || { echo "Error: bound camera object did not return allowed." >&2; exit 1; }
json_has_string "${authorized_result}" zoneId SMOKE-ZONE || { echo "Error: allowed decision returned the wrong zoneId." >&2; exit 1; }
json_has_string "${authorized_result}" employeeId SMOKE-EMPLOYEE || { echo "Error: allowed decision returned the wrong employeeId." >&2; exit 1; }

echo "[8/8] Cleanup"
cleanup
trap - EXIT

echo "SecureZone runtime camera-identity smoke test passed."
