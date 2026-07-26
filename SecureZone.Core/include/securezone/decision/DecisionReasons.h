#pragma once

namespace securezone::decision::DecisionReasons {

inline constexpr const char* PersonOutsideZone = "No active zone entry event.";
inline constexpr const char* PendingIdentity = "Waiting for identity association.";
inline constexpr const char* UnknownIdentity = "Person identity is unknown.";
inline constexpr const char* InactiveEmployee = "Employee is inactive.";
inline constexpr const char* InactiveZoneAccessRules = "Zone is inactive for access rules.";
inline constexpr const char* SafeZoneAccess = "Identified employee is inside a safe zone.";
inline constexpr const char* MachineStateDenied = "Machine state does not allow access.";
inline constexpr const char* RoleDenied = "Employee role is not allowed in this zone.";
inline constexpr const char* AccessAllowed = "Employee is allowed in this zone.";

}
