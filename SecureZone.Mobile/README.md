# SecureZone Mobile

React Native / Expo presentation layer for the SecureZone mobile application.

## Visual screen coverage

- shared: splash, login, password recovery, profile, loading, empty, offline, error and permission states;
- scanner: camera/QR scanner preview, accepted/denied check-in results, history and station settings;
- worker: home, personal QR, zone list/details and safety briefing;
- manager: dashboard, alarm list/details, camera view, operations, event history, people, maintenance, machine details and notifications;
- admin: overview, identity administration, employee details, system configuration, policy details and audit log.

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
