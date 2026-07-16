# SecureZone Mobile

React Native / Expo presentation layer for the SecureZone mobile application.

## Visual screen coverage

- shared: login, profile, loading, empty, offline, error and permission states;
- scanner: camera/QR scanner preview and accepted/denied check-in results;
- worker: personal QR access card and permitted/restricted zone access;
- manager: dashboard, active alarm details and operations overview;
- admin: identity administration and system configuration.

All employee records, QR patterns, alarms, metrics, filters, cameras, controls and navigation bars are
static presentation fixtures. Authentication, API calls, camera access, QR token generation, secure
storage, push notifications, navigation, role routing and backend integrations are intentionally out
of scope.

`App.tsx` selects one exported screen as the active visual preview. Every completed screen is exported
from `src/screens/index.ts` and can be selected there without introducing navigation logic.

## Local preview

```powershell
pnpm install
pnpm start
```
