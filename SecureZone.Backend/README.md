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
.\build\SecureZone.Backend\Debug\SecureZoneBackend.exe --mode file --config .\securezone.backend.json
```

## Options

- `--mode file` selects local metadata file processing mode.
- `--config <path>` points to a future backend config file.
- `--dry-run` validates startup options without running processing.
- `--help` prints usage.
