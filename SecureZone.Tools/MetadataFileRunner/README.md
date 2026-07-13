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

The runner prints a grouped summary:

- input camera/file settings
- metadata processing counters
- decision engine counters
- in-memory storage counters
- created/resolved alarms

Example:

```text
== Input ==
Camera ID: CAM-001
Metadata file: C:\path\to\metadata.xml
Identity grace period: disabled

== Processing ==
Detections processed: 1
Tracks upserted: 1
Events created: 1

== Decision ==
Detections checked: 1
Decisions evaluated: 1
Allowed: 0
Pending identity: 0
Violations: 1
Ignored: 0

== Storage ==
Stored tracks: 1
Stored metadata events: 1

== Alarms ==
Alarms created: 1
Alarms resolved: 0
```
