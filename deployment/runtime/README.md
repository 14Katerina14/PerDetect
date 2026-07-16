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
the XProtect plug-in. The MongoDB username used in `SECUREZONE_MONGO_URI` must
match `MONGO_INITDB_ROOT_USERNAME`.

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
starts both services and displays their status. The logs scripts follow the
last 200 lines. Pass either `mongodb` or `securezone-api` to follow one service;
omit the argument to follow both.

Check API health directly with:

```powershell
Invoke-RestMethod http://127.0.0.1:8080/health
```

```bash
curl --fail --silent --show-error http://127.0.0.1:8080/health
```

Replace `8080` if `SECUREZONE_API_HOST_PORT` is different.

## Camera identity and access-decision flow

The backend does not authorize a person from a zone-level presence record
alone. The runtime flow is:

1. XProtect sends a recent `Human` observation with `cameraId` and `objectId`.
2. The QR scanner submits the employee, zone, scanner user, and the same
   `cameraId`.
3. The backend binds the employee to the latest unbound Human object from that
   camera for the lifetime of the presence session.
4. XProtect sends LineCrossing with the same `cameraId` and `objectId`.
5. The backend evaluates employee roles, zone policy, machine state, and the
   active identity binding, then returns `allowed` or `violation`.

Do not omit `cameraId` from QR requests or `cameraId`/`objectId` from
LineCrossing events. They are required identity evidence.

## Run the repeatable smoke test

The smoke scripts create temporary, isolated records named `SMOKE-*` and use
camera `CAM-SMOKE`. They do not modify the normal production/demo seed data.
Each run verifies:

1. `GET /health` returns `status: ok`.
2. An unknown camera object entering the smoke zone returns `violation`.
3. The same object exiting returns `cleared`, so no test alarm remains active.
4. A recent Human observation is accepted for `CAM-SMOKE`.
5. QR check-in includes `cameraId`, binds `SMOKE-EMPLOYEE` to the observed
   object, and returns a non-empty `bindingId`.
6. LineCrossing for the bound `cameraId`/`objectId` returns `allowed` for the
   isolated maintenance employee while the isolated machine is `stopped`.
7. Temporary employees, scanner users, machines, zones, policies, sessions,
   tracks, bindings, and alarms are removed.

```powershell
./scripts/smoke-test.ps1
```

```bash
./scripts/smoke-test.sh
```

The Bash script uses an exit trap and the PowerShell script uses `try/finally`,
so cleanup is attempted after both success and failure. The scripts stop on
HTTP, MongoDB, or JSON validation errors and do not print the API key or the
MongoDB password.

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
Allow the configured API host port (default `8080`) through the host firewall
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
docker compose --env-file .env -f docker-compose.yml up -d --build
docker compose --env-file .env -f docker-compose.yml ps
docker compose --env-file .env -f docker-compose.yml logs --tail 200 -f
docker compose --env-file .env -f docker-compose.yml down
```

The stop command intentionally omits `-v` so MongoDB data remains persistent.
