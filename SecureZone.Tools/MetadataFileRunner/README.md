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

Optional JSON output:

```powershell
.\build\SecureZone.Tools\MetadataFileRunner\Debug\SecureZoneMetadataFileRunner.exe --camera-id CAM-001 --metadata-file C:\path\to\metadata.xml --output-json C:\path\to\result.json
```

## Repeatable sample fixtures

The repository includes camera-free ONVIF metadata samples in `fixtures/`.
Run these commands from the repository root after building the runner:

```powershell
# Expected: one violation and one alarm.
.\build\SecureZone.Tools\MetadataFileRunner\Debug\SecureZoneMetadataFileRunner.exe --camera-id CAM-001 --metadata-file .\SecureZone.Tools\MetadataFileRunner\fixtures\person_inside_dangerous_zone.xml

# Expected: detected person, but no alarm because the person is outside the demo zone.
.\build\SecureZone.Tools\MetadataFileRunner\Debug\SecureZoneMetadataFileRunner.exe --camera-id CAM-001 --metadata-file .\SecureZone.Tools\MetadataFileRunner\fixtures\person_outside_dangerous_zone.xml

# Expected: no tracks, decisions, or alarms.
.\build\SecureZone.Tools\MetadataFileRunner\Debug\SecureZoneMetadataFileRunner.exe --camera-id CAM-001 --metadata-file .\SecureZone.Tools\MetadataFileRunner\fixtures\empty_no_person_metadata.xml
```

The demo dangerous zone is the rectangle from `(0, 0)` to `(10000, 10000)`.
The fixture coordinates are deliberately chosen around that boundary.

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

When `--output-json` is provided, the same result is also written to a machine-readable JSON file:

```json
{
  "cameraId": "CAM-001",
  "metadataFile": "C:\\path\\to\\metadata.xml",
  "identityGracePeriodActive": false,
  "processing": {
    "detectionsProcessed": 1,
    "tracksUpserted": 1,
    "eventsCreated": 1
  },
  "decision": {
    "detectionsChecked": 1,
    "decisionsEvaluated": 1,
    "allowed": 0,
    "pendingIdentity": 0,
    "violations": 1,
    "ignored": 0,
    "alarmsCreated": 1,
    "alarmsResolved": 0
  },
  "storage": {
    "storedTracks": 1,
    "storedMetadataEvents": 1
  },
  "alarms": [
    {
      "alarmId": "ALARM-001",
      "trackId": "TRACK-001",
      "zoneId": "ZONE-DEMO",
      "employeeId": "",
      "machineId": "MACHINE-DEMO",
      "status": "created",
      "reason": "missing_identity",
      "message": "Person is inside a dangerous zone without confirmed identity.",
      "stillInside": true
    }
  ]
}
```
