# SecureZone backend container

This directory builds the C++ SecureZone API as a Linux container. The image
contains the HTTP runtime and MongoDB infrastructure, runs as a non-root user,
and exposes port `8080`.

## Build

Run from the repository root:

```powershell
docker build --file deployment/backend/Dockerfile --tag securezone-api:local .
```

The first build compiles the MongoDB C++ driver through vcpkg and can take
several minutes. Later source-only builds reuse the dependency layer.

## Run against an existing MongoDB container

Create a local `.env` from `.env.example` and replace every placeholder. Never
commit the resulting `.env` file.

```powershell
Copy-Item deployment/backend/.env.example deployment/backend/.env
docker run --rm --name securezone-api --env-file deployment/backend/.env -p 8080:8080 securezone-api:local
```

When MongoDB runs in another container, both containers must share a Docker
network and the URI hostname must match the MongoDB service name. The runtime
compose deployment owns that network configuration.

## Verify

```powershell
Invoke-RestMethod http://localhost:8080/health
docker inspect --format '{{.State.Health.Status}}' securezone-api
```

The expected health response is:

```json
{"status":"ok","service":"securezone-api"}
```

## Runtime contract

The image starts `/app/SecureZone.Api` and accepts these environment variables:

| Variable | Default | Purpose |
| --- | --- | --- |
| `SECUREZONE_API_HOST` | `0.0.0.0` | Listener address inside the container |
| `SECUREZONE_API_PORT` | `8080` | Listener port inside the container |
| `SECUREZONE_MONGO_URI` | none | Complete MongoDB connection URI |
| `SECUREZONE_MONGO_DATABASE` | `securezone` | MongoDB database name |
| `SECUREZONE_XPROTECT_API_KEY` | none | Shared key accepted from the XProtect plug-in |
| `SECUREZONE_QR_PRESENCE_MINUTES` | `2` | QR presence validity duration |
| `SECUREZONE_UNIDENTIFIED_GRACE_SECONDS` | `120` | Time allowed to bind a visible Human object to a QR identity |

Secrets are runtime environment values. They are not copied into the image.
