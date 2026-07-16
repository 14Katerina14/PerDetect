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

## Run the repeatable smoke test

The smoke test uses the records seeded by
`deployment/mongodb/init/01-init-securezone.js` and verifies:

1. `GET /health` returns `status: ok`.
2. A unique XProtect LineCrossing event returns `decision: violation` before
   QR presence exists.
3. The seeded employee `EMP-001` is checked into `ZONE-001` by
   `APP-SCANNER-001`, and the response is accepted.
4. A second event with a different ID returns `decision: allowed`.

```powershell
./scripts/smoke-test.ps1
```

```bash
./scripts/smoke-test.sh
```

The scripts stop immediately on HTTP or JSON-field validation failures and do
not print the API key.

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
