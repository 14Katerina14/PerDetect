# SecureZone Core

Pure C++ business-domain library for SecureZone. It deliberately has no database,
network, MIP SDK, C++/CLI, frontend, or deployment dependency.

The domain covers normalized camera detections, polygonal zones, employees and QR
check-ins, track-to-identity association, machine-aware access policies, decision
results, role UI permissions, and alarm lifecycle transitions.

Example flow: a person detection inside a dangerous zone is associated to one valid
QR check-in for that zone, evaluated against the employee role and machine state,
then creates an alarm that can become active, acknowledged, and resolved on exit.
