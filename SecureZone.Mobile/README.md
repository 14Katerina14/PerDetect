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

Create an installable Android preview APK with an Expo account:

```powershell
npx eas-cli@latest build --platform android --profile preview
```
