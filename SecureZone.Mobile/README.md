# SecureZone Mobile

Real Expo/React Native client for the SecureZone backend.

## Implemented flow

- login through `POST /api/auth/login`;
- JWT session stored in the platform secure store;
- scanner role opens the physical camera and sends QR check-ins;
- worker role displays a generated employee QR code;
- manager/admin roles read active and recent alarms every three seconds;
- sign-out clears the local JWT session.

The scanner station requires the XProtect `zoneId` and camera GUID. They can be
entered on the scanner screen and are stored locally, or baked into an APK with:

```text
EXPO_PUBLIC_SECUREZONE_ZONE_ID=ZONE-001
EXPO_PUBLIC_SECUREZONE_CAMERA_ID=CAMERA-GUID
EXPO_PUBLIC_SECUREZONE_API_URL=http://MYIP:18080
```

For a physical phone, `MYIP` must be the backend laptop IPv4 address reachable
from the same Wi-Fi. `localhost` on the phone is the phone itself and will not
reach the backend.

## Local checks

```powershell
cd SecureZone.Mobile
pnpm install --frozen-lockfile
pnpm check
pnpm start
```

An installed release APK contains the JavaScript bundle and does not need Metro.
It still needs network access from the phone to the configured SecureZone API.
Runtime updates are disabled, so the installed APK has no Expo or Internet dependency.

Create an installable Android preview APK with an Expo account:

```powershell
npx eas-cli@latest build --platform android --profile preview
```

## Test at home without XProtect

The test server uses the same HTTP paths and response shapes as the production
API. It replaces only the external XProtect/Mongo source while testing the real
mobile networking, login, QR camera, role routing and alarm screens.

```powershell
cd SecureZone.Mobile
node scripts/mock-api.mjs
```

Find the laptop Wi-Fi IPv4 address with `ipconfig`, allow TCP 19080 through the
Windows firewall, and enter `http://LAPTOP_IP:19080` on the phone. Press **Test
server** before signing in. Test accounts use password `demo`:

| Username | Screen |
|---|---|
| `worker` | Employee QR |
| `scanner` | Physical QR scanner |
| `manager` | Active and recent alarms |
| `admin` | Active and recent alarms |

For a one-phone test, keep the phone logged in as `scanner` and display the
worker QR on the laptop:

```powershell
node scripts/generate-worker-qr.mjs EMP-001
Start-Process .\worker-EMP-001-qr.png
```

Raise and clear an alarm from another PowerShell window:

```powershell
Invoke-RestMethod -Method Post http://LAPTOP_IP:19080/test/alarm/raise
Invoke-RestMethod -Method Post http://LAPTOP_IP:19080/test/alarm/clear
```

The automated local contract check is:

```powershell
.\scripts\test-mock-flow.ps1
```

## Office network checklist

1. The backend binds to `0.0.0.0:18080` and Docker publishes port 18080.
2. Windows Firewall allows inbound TCP 18080 on the backend laptop.
3. The XProtect plug-in uses the backend laptop LAN IPv4 address.
4. Phones use either the reachable company Wi-Fi address or the backend laptop
   hotspot address `http://YOUR-LAPTOP-IP:18080`.
5. `/health` and `/version` must pass from both the phone and XProtect laptop.
6. A QR retry is valid for only 30 seconds; expired scans must be repeated while
   the worker remains visible to the camera.
