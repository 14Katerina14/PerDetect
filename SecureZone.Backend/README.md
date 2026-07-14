# SecureZone Backend

SecureZone backend service runner.

This executable is the application entry point that wires config, repositories,
metadata input, processing, decisions, alarm persistence, and webhook delivery
records.

The current version supports local metadata file input with in-memory metadata
and decision repositories. MongoDB and XProtect live stream wiring will be added
later.

## Structure

- `main.cpp` owns command-line parsing only.
- `BackendCompositionRoot` loads configuration and creates the application.
- `BackendApplication` owns the runtime flow that later commits will extend
  with metadata input, service wiring, repositories, and webhook delivery
  records.

## Build

```powershell
cmake -S . -B build -DSECUREZONE_BUILD_MONGODB_INFRA=OFF
cmake --build build --config Debug --target SecureZoneBackend
```

## Run

```powershell
copy .\SecureZone.Backend\securezone.backend.example.json .\securezone.backend.json
.\build\SecureZone.Backend\Debug\SecureZoneBackend.exe --mode file --config .\securezone.backend.json --dry-run
.\build\SecureZone.Backend\Debug\SecureZoneBackend.exe --mode file --config .\securezone.backend.json
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

## File Metadata Mode

In `metadataInputMode: "file"`, the backend reads `metadataFilePath`, processes
the XML through the core metadata and decision pipeline, and prints:

- detections processed
- tracks upserted
- events created
- stored tracks
- stored metadata events
- decisions evaluated
- allowed / pending identity / violations
- alarms created / resolved

This mode uses a demo dangerous zone from `(0, 0)` to `(10000, 10000)`, a demo
running machine, and in-memory repositories until MongoDB wiring is added.
