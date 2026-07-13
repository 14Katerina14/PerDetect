# SecureZone Metadata File Runner

Small local demo runner for processing a raw ONVIF/XProtect metadata XML file through the SecureZone backend flow.

The runner uses in-memory repositories, so it does not need MongoDB or the MIP/.NET bridge.

## Build

```powershell
cmake -S . -B build -DSECUREZONE_BUILD_MONGODB_INFRA=OFF
cmake --build build --config Debug --target SecureZoneMetadataFileRunner
```

## Run

```powershell
.\build\SecureZone.Tools\MetadataFileRunner\Debug\SecureZoneMetadataFileRunner.exe --camera-id CAM-001 --metadata-file C:\path\to\metadata.xml
```

Optional identity grace period mode:

```powershell
.\build\SecureZone.Tools\MetadataFileRunner\Debug\SecureZoneMetadataFileRunner.exe --camera-id CAM-001 --metadata-file C:\path\to\metadata.xml --identity-grace-period
```

## Output

The runner prints a summary of detections, persisted tracks/events, decisions, violations, and alarms.
