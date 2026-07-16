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
| `SECUREZONE_JWT_SECRET` | none | HS256 signing secret; required and at least 32 bytes |
| `SECUREZONE_JWT_TTL_MINUTES` | `60` | Mobile access-token lifetime |

Secrets are runtime environment values. They are not copied into the image.

The API refuses to start when `SECUREZONE_JWT_SECRET` is missing or shorter
than 32 bytes. Use an independently generated random value and never commit it.

## Login and endpoint authorization

`POST /api/auth/login` accepts a JSON object containing `username` and
`password`. Successful login returns an HS256 JWT for the
`securezone-mobile` audience. Send it as `Authorization: Bearer <token>`.

| Endpoint | Protection |
| --- | --- |
| `GET /health` | Public |
| `POST /api/auth/login` | Public |
| `POST /api/qr/check-in` | Scanner JWT only |
| `POST /api/xprotect/line-crossing` | `X-SecureZone-Api-Key`; no JWT |
| `POST /api/xprotect/object-observations` | `X-SecureZone-Api-Key`; no JWT |

The QR request body must not be trusted for scanner identity. The backend uses
the JWT `sub` claim as `scannedByUserId`; a client-provided value is ignored.
Application roles (`worker`, `scanner`, `manager`, `admin`) do not grant
physical zone access. Zone decisions continue to use employee roles and access
policies independently.

## Provision an application-user password

Passwords are stored only as libsodium Argon2id hashes in
`app_users.passwordHash`. The hash tool reads one password from standard input
and rejects command-line arguments so the password is not exposed in the
process list.

After building the image, run:

```text
docker run --rm -i --entrypoint /app/SecureZone.PasswordHash securezone-api:local
```

Provide the password on standard input and copy only the generated
`$argon2id$...` output. Store that output in MongoDB using an administrative
workflow, for example by setting `passwordHash` on the selected `app_users`
record. Do not put either the password or generated hash in source control,
shell history, `.env`, documentation, or logs.

A Worker account must also have `employeeId`. Manager and Admin accounts may
have one, while the dedicated Scanner account may omit it. Missing or malformed
hashes, inactive users, and Worker accounts without `employeeId` fail closed as
`invalid_credentials`.
