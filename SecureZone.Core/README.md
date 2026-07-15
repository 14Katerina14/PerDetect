# SecureZone Core

Pure C++ business-domain library for SecureZone. It deliberately has no database,
network, MIP SDK, C++/CLI, frontend, or deployment dependency.

The domain covers XProtect/WiseAI zone entry events, zones mapped to XProtect
event names, employees and QR check-ins, machine-aware access policies, decision
results, role UI permissions, and alarm lifecycle transitions.

Example flow: a WiseAI line-crossing event is normalized as a zone entry event,
matched to a configured zone and current QR/presence state, evaluated against the
employee role and machine state, then creates an alarm that can become active,
acknowledged, and resolved.

## Decision and alarm flow

`DecisionEngine` evaluates a `DecisionContext` and returns an `AccessDecision`.
The result includes both the business outcome and the required alarm action:

- `PendingIdentity` while the QR-to-track identity grace period is active;
- `UnknownIdentity` or `Violation` when an alarm must be created;
- `Allowed` with `shouldClearAlarm` when no zone entry event is active for a zone
  that has an active alarm.

`AlarmPersistenceService` uses `IAlarmRepository` to create at most one active
alarm for a track and zone, and resolves it when the decision requests clearing.
An alarm records its decision reason and resolution timestamp. The core test suite
covers these transitions through an in-memory repository fake.
