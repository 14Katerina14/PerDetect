# SecureZone XProtect Decision Bridge

This MIP plugin runs inside the XProtect Event Server on the company XProtect
machine. It is the bridge between Hanwha WiseAI LineCrossing events and the
SecureZone C++ backend.

## Runtime flow

```text
Human appears in the camera field of view
  -> XProtect metadata stream exposes cameraId + ObjectId + Human
  -> plugin POST /api/xprotect/object-observations
  -> scanner phone POST /api/qr/check-in with the same cameraId
  -> backend binds the employee to the newest unbound Human ObjectId
WiseAI LineCrossing for that object
  -> XProtect Event Server
  -> plugin correlates the XProtect event with raw metadata ObjectId
  -> plugin POST /api/xprotect/line-crossing with cameraId + ObjectId
  -> backend checks the MongoDB zone and active object identity binding
  -> allowed: plugin logs the decision and raises no event
  -> violation: plugin raises SecureZoneViolationConfirmed
  -> XProtect rule activates the external speaker/output device
```

The plugin does not contain employee, QR, zone, or permission business logic.
It trusts only a successful backend response where `accepted` is `true` and
`decision` is `violation`.

Backend timeout, HTTP errors, invalid JSON, unknown zones, and `allowed`
responses never raise `SecureZoneViolationConfirmed`. They are written to the
XProtect Event Server log.

## Backend contract

The plugin sends:

```json
{
  "eventId": "xprotect-event-guid",
  "eventName": "Channel.<int>.OpenSDK.WiseAI.LineCrossing.<int>.State-2",
  "sourceName": "Camera 1",
  "receivedAt": "2026-07-15T10:30:00.0000000Z",
  "cameraId": "camera-guid",
  "objectId": "42",
  "action": "Crossed"
}
```

For every Human object received from the live metadata stream, the plugin also
sends:

```json
{
  "cameraId": "camera-guid",
  "objectId": "42",
  "objectType": "Human",
  "observedAt": "2026-07-15T10:29:55.0000000Z"
}
```

to `POST /api/xprotect/object-observations`. The MIP SDK related camera item is
used as `cameraId`. The scanner app must send that camera GUID in its QR
check-in request. The default QR matching window is 15 seconds, so the employee
should scan immediately after entering the camera field of view.

It optionally sends `X-SecureZone-Api-Key` when
`SECUREZONE_XPROTECT_API_KEY` is configured.

The event is confirmed only for a response shaped like:

```json
{
  "accepted": true,
  "status": "processed",
  "decision": "violation",
  "zoneId": "ZONE-001",
  "message": "Camera object has no active QR identity binding."
}
```

## Build

Build `SecureZone.XProtectPlugin.csproj` in Release mode on a Windows machine
that can restore the `MilestoneSystems.VideoOS.Platform` package:

```powershell
msbuild .\SecureZone.XProtectPlugin.csproj /t:Restore,Build /p:Configuration=Release
```

Place these files together before running the installer:

```text
SecureZone.XProtectPlugin.dll
plugin.def
install-on-xprotect-server.ps1
```

## Install on the XProtect Event Server machine

Run PowerShell as Administrator. Use the actual reachable backend address and
the same API key configured as `SECUREZONE_XPROTECT_API_KEY` for the backend:

```powershell
.\install-on-xprotect-server.ps1 `
  -ApiUrl "http://SECUREZONE-BACKEND:8080/api/xprotect/line-crossing" `
  -ApiKey "ENTER-AT-INSTALL-TIME"
```

The key is supplied at installation time and must not be committed to Git.

## XProtect rule

Do not configure the old raw LineCrossing rule action named
`SecureZone: confirm violation`. Version 2 removes that action because it
bypassed the backend QR decision.

Create the output/speaker rule with this trigger:

```text
Class: SecureZone
Type: SecureZoneViolationConfirmed
Name: SecureZone violation confirmed
```

The rule action should activate the external speaker or output device. The
plugin raises this event only after the backend returns `violation`.
