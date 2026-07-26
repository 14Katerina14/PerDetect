# SecureZone Docker Compose runtime

This directory runs the SecureZone C++ API and MongoDB together for local or
company-host deployment. MongoDB stays bound to the Docker host loopback
interface by default, while the API port is published for the XProtect plug-in.

## Prerequisites

- Docker Desktop, or Docker Engine with Docker Compose v2
- PowerShell 7+ for the `.ps1` scripts, or Bash and `curl` for the `.sh` scripts
- Enough free disk space and time for the first C++ image build (vcpkg compiles
  the MongoDB C++ dependencies during that build)

Run all commands below from `deployment/runtime` unless noted otherwise.

## Configure the environment

Create a local environment file and replace every placeholder:

```powershell
Copy-Item .env.example .env
```

```bash
cp .env.example .env
```

Never commit `.env`. Use a long random value for
`SECUREZONE_XPROTECT_API_KEY`, and configure the same value in the backend and
the XProtect plug-in. Generate a separate random value of at least 32 bytes for
`SECUREZONE_JWT_SECRET`; it signs mobile/API access tokens and must never be
shared with the plug-in. The MongoDB username used in
`SECUREZONE_MONGO_URI` must match `MONGO_INITDB_ROOT_USERNAME`.

If the MongoDB password contains reserved characters such as `@`, `:`, `/`,
`?`, `#`, `[` or `]`, URL-encode the password inside `SECUREZONE_MONGO_URI`.
The root user authenticates against the `admin` database, so keep
`authSource=admin` in the URI.

## Start and inspect the deployment

PowerShell:

```powershell
./scripts/start.ps1
./scripts/logs.ps1
./scripts/logs.ps1 securezone-api
```

Bash:

```bash
./scripts/start.sh
./scripts/logs.sh
./scripts/logs.sh securezone-api
```

The start script first runs `docker compose config --quiet`, then builds and
starts both services, waits until their health checks pass, and displays their
status. The logs scripts follow the last 200 lines. Pass either `mongodb` or
`securezone-api` to follow one service; omit the argument to follow both.

Check API health directly with:

```powershell
Invoke-RestMethod http://127.0.0.1:18080/health
```

```bash
curl --fail --silent --show-error http://127.0.0.1:18080/health
```

Replace `18080` if `SECUREZONE_API_HOST_PORT` is different.

Confirm which backend image is running with:

```powershell
Invoke-RestMethod http://127.0.0.1:18080/version
docker compose logs --follow securezone-api
```

Set `SECUREZONE_BUILD_ID` in `.env` to the short Git commit or another release
identifier before building the image. API access logs include method, path,
status, duration, remote address, and `eventId` when the request contains one.
Request bodies, authorization headers, passwords, JWTs, and API keys are not
logged.

## Camera identity and access-decision flow

The backend does not authorize a person from a zone-level presence record
alone. The runtime flow is:

1. XProtect sends a recent `Human` observation with `cameraId` and `objectId`.
2. The smoke test securely provisions a temporary scanner account, logs in,
   and obtains a JWT.
3. The authenticated QR scanner submits the employee, zone, and the same
   `cameraId`; the backend takes the scanner identity from the JWT.
4. The backend binds the employee to the latest unbound Human object from that
   camera for the lifetime of the presence session.
5. XProtect sends LineCrossing with the same `cameraId` and `objectId`.
6. The backend evaluates employee roles, zone policy, machine state, and the
   active identity binding, then returns `allowed` or `violation`.

Do not omit `cameraId` from QR requests or `cameraId`/`objectId` from
LineCrossing events. They are required identity evidence.

## Alarm read endpoints

The mobile manager/admin view can read alarms with a valid Bearer JWT:

```text
GET /api/alarms/active
GET /api/alarms/recent
```

`active` returns up to 50 active or acknowledged alarms. `recent` returns up
to 50 alarms across the complete lifecycle, newest first, including resolved
alarms. Both responses include employee and zone names when available, the
violation reason, timestamps, and `stillInside`. Scanner and worker accounts
receive `403`; missing or invalid credentials receive `401`.

## Run the full local event injector

The PowerShell injector creates temporary, isolated records named `SMOKE-*`
and uses camera `CAM-SMOKE`. It does not modify the normal production/demo
seed data. The requests use the normal HTTP routes and application services;
only XProtect and the physical camera are replaced by the script. Each run
verifies:

1. `GET /health` returns `status: ok`.
2. `GET /version` identifies the running backend image.
3. A temporary scanner is provisioned with Argon2id, logs in, and receives a
   JWT without storing a password or password hash in the repository.
4. A recent Human observation is accepted for `CAM-SMOKE`.
5. Authenticated QR check-in binds `SMOKE-EMPLOYEE` to the latest Human object
   and returns a non-empty `bindingId`.
6. The bound worker enters a forbidden zone and the real policy engine returns
   `violation` because the worker role is not allowed.
7. MongoDB contains one active alarm and one pending manager notification,
   and the manager JWT can read it through `/api/alarms/active`.
8. Replaying the same XProtect `eventId` is idempotent and creates no duplicate
   alarm or notification.
9. The worker exits, the API returns `cleared`, the alarm is resolved, a
   separate manager clear notification is queued, and `/api/alarms/recent`
   exposes the resolved lifecycle state.
10. Temporary employees, app users, subscriptions, machines, zones, policies,
    sessions, tracks, bindings, alarms, and notifications are removed.

```powershell
./scripts/smoke-test.ps1
```

The script uses `try/finally`, so cleanup is attempted after both success and
failure. It stops on HTTP, MongoDB, or JSON validation errors and does not
print the API key, MongoDB password, scanner password, or JWT.

`./scripts/smoke-test.sh` remains available as the smaller Linux deployment
smoke test. Use the PowerShell injector above to validate the complete alarm
and notification lifecycle.

## Stop and preserve data

The normal stop scripts remove the containers and network but preserve the
named volume `securezone-mongo-data`:

```powershell
./scripts/stop.ps1
```

```bash
./scripts/stop.sh
```

For an explicit development-only database reset, stop the runtime and delete
the volume manually. This permanently removes the local MongoDB data:

```powershell
docker compose --env-file .env down -v
```

The initialization script runs only when MongoDB starts with an empty data
volume. A normal restart keeps existing collections and records.

## XProtect connectivity and ports

Configure the XProtect plug-in endpoint as:

```text
http://<docker-host-ip>:<SECUREZONE_API_HOST_PORT>/api/xprotect/line-crossing
```

Use the Docker host machine's reachable IP address. Do not use `localhost`
unless XProtect Event Server and the Docker backend run on the same machine.
Allow the configured API host port (default `18080`) through the host firewall
only for the machines that need it.

MongoDB's container port `27017` is published as host port `27018` by default,
but only on `127.0.0.1`. Do not change `MONGO_BIND_ADDRESS` to a LAN address and
do not expose MongoDB to the company network. The API connects privately to the
`mongodb` service over `securezone-runtime-network`; XProtect never needs the
MongoDB port.

## Manual Compose commands

The scripts are wrappers around these commands:

```text
docker compose --env-file .env -f docker-compose.yml config --quiet
docker compose --env-file .env -f docker-compose.yml up -d --build --wait --wait-timeout 180
docker compose --env-file .env -f docker-compose.yml ps
docker compose --env-file .env -f docker-compose.yml logs --tail 200 -f
docker compose --env-file .env -f docker-compose.yml down
```

The stop command intentionally omits `-v` so MongoDB data remains persistent.
