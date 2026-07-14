# SecureZone Backend

SecureZone backend service runner.

This executable will become the application entry point that wires config,
repositories, metadata input, processing, decisions, alarm persistence, and
webhook delivery records.

The current version is a skeleton only. It validates command-line arguments and
prints the selected runtime mode without connecting to MongoDB or XProtect yet.

## Build

```powershell
cmake -S . -B build -DSECUREZONE_BUILD_MONGODB_INFRA=OFF
cmake --build build --config Debug --target SecureZoneBackend
```

## Run

```powershell
copy .\SecureZone.Backend\securezone.backend.example.json .\securezone.backend.json
.\build\SecureZone.Backend\Debug\SecureZoneBackend.exe --mode file --config .\securezone.backend.json --dry-run
```

## Options

- `--mode file` selects local metadata file processing mode.
- `--config <path>` points to a future backend config file.
- `--dry-run` validates startup options without running processing.
- `--help` prints usage.

## Configuration

Use `securezone.backend.example.json` as a template. The real
`securezone.backend.json` file is ignored by git and should not be committed.

The MongoDB connection string is referenced by environment variable name:

```json
{
  "mongoConnectionStringEnv": "SECUREZONE_MONGO_URI",
  "mongoDatabaseName": "securezone",
  "cameraId": "CAM-001",
  "metadataInputMode": "file",
  "metadataFilePath": "SecureZone.Tools/MetadataFileRunner/fixtures/person_inside_dangerous_zone.xml"
}
```

Store the actual MongoDB URI in `SECUREZONE_MONGO_URI`, not in the config file.
