#include "securezone/decision/DecisionEngine.h"

#include <cassert>
#include <chrono>
#include <functional>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "securezone/decision/DecisionReasons.h"

namespace {

using namespace securezone;

domain::ZoneEntryEvent makeZoneEntryEvent() {
    domain::ZoneEntryEvent event{};
    event.eventId = "event-1";
    event.trackId = "track-1";
    event.cameraId = "camera-1";
    event.sourceName = "Hanwha Vision TNO-C4052T";
    event.timestamp = std::chrono::system_clock::now();
    return event;
}

domain::Zone makeZone(
    domain::ZoneType type = domain::ZoneType::Dangerous,
    domain::ZoneStatus status = domain::ZoneStatus::Active
) {
    domain::Zone zone{};
    zone.zoneId = "zone-1";
    zone.name = "Dangerous Zone";
    zone.cameraId = "camera-1";
    zone.type = type;
    zone.status = status;
    zone.relatedMachineId = "machine-1";
    return zone;
}

domain::Employee makeEmployee(
    std::vector<std::string> roles = {"maintenance"},
    domain::EmployeeStatus status = domain::EmployeeStatus::Active
) {
    domain::Employee employee{};
    employee.employeeId = "employee-1";
    employee.fullName = "Ivan Petrov";
    employee.roles = std::move(roles);
    employee.status = status;
    return employee;
}

domain::MachineState makeMachineState(
    domain::MachineStatus status = domain::MachineStatus::Stopped
) {
    domain::MachineState machineState{};
    machineState.machineId = "machine-1";
    machineState.status = status;
    return machineState;
}

domain::AccessPolicy makeAccessPolicy(
    std::vector<std::string> allowedRoles = {"maintenance"},
    std::vector<domain::MachineStatus> allowedMachineStates = {domain::MachineStatus::Stopped}
) {
    domain::AccessPolicy policy{};
    policy.policyId = "policy-1";
    policy.zoneId = "zone-1";
    policy.allowedRoles = std::move(allowedRoles);
    policy.machineStatesAllowed = std::move(allowedMachineStates);
    return policy;
}

decision::DecisionContext makeContext(
    const domain::ZoneEntryEvent& zoneEntryEvent,
    const domain::Zone& zone,
    const std::optional<std::reference_wrapper<const domain::Employee>>& employee,
    const domain::MachineState& machineState,
    const domain::AccessPolicy& accessPolicy
) {
    decision::DecisionContext context{
        zoneEntryEvent,
        zone,
        employee,
        machineState,
        accessPolicy
    };
    context.hasZoneEntryEvent = true;
    return context;
}

void allowsWhenNoZoneEntryEventAndClearsExistingAlarm() {
    const auto event = makeZoneEntryEvent();
    const auto zone = makeZone();
    const auto employee = makeEmployee();
    const auto machineState = makeMachineState();
    const auto policy = makeAccessPolicy();
    auto context = makeContext(event, zone, std::cref(employee), machineState, policy);
    context.hasZoneEntryEvent = false;
    context.hadActiveAlarm = true;

    const auto result = decision::DecisionEngine{}.evaluate(context);

    assert(result.type == domain::AccessDecisionType::Allowed);
    assert(result.reason == decision::DecisionReasons::PersonOutsideZone);
    assert(!result.shouldCreateAlarm);
    assert(result.shouldClearAlarm);
}

void returnsPendingIdentityWhileGracePeriodIsActive() {
    const auto event = makeZoneEntryEvent();
    const auto zone = makeZone();
    const auto machineState = makeMachineState();
    const auto policy = makeAccessPolicy();
    auto context = makeContext(event, zone, std::nullopt, machineState, policy);
    context.isIdentityGracePeriodActive = true;

    const auto result = decision::DecisionEngine{}.evaluate(context);

    assert(result.type == domain::AccessDecisionType::PendingIdentity);
    assert(result.reason == decision::DecisionReasons::PendingIdentity);
    assert(!result.shouldCreateAlarm);
}

void createsUnknownIdentityAlarmAfterGracePeriodExpires() {
    const auto event = makeZoneEntryEvent();
    const auto zone = makeZone(domain::ZoneType::Safe);
    const auto machineState = makeMachineState();
    const auto policy = makeAccessPolicy();
    const auto context = makeContext(event, zone, std::nullopt, machineState, policy);

    const auto result = decision::DecisionEngine{}.evaluate(context);

    assert(result.type == domain::AccessDecisionType::UnknownIdentity);
    assert(result.reason == decision::DecisionReasons::UnknownIdentity);
    assert(result.shouldCreateAlarm);
}

void createsUnknownIdentityAlarmInInactiveZoneAfterGracePeriodExpires() {
    const auto event = makeZoneEntryEvent();
    const auto zone = makeZone(domain::ZoneType::Safe, domain::ZoneStatus::Inactive);
    const auto machineState = makeMachineState();
    const auto policy = makeAccessPolicy();
    const auto context = makeContext(event, zone, std::nullopt, machineState, policy);

    const auto result = decision::DecisionEngine{}.evaluate(context);

    assert(result.type == domain::AccessDecisionType::UnknownIdentity);
    assert(result.reason == decision::DecisionReasons::UnknownIdentity);
    assert(result.shouldCreateAlarm);
}

void allowsIdentifiedEmployeeInInactiveZone() {
    const auto event = makeZoneEntryEvent();
    const auto zone = makeZone(domain::ZoneType::Dangerous, domain::ZoneStatus::Inactive);
    const auto employee = makeEmployee();
    const auto machineState = makeMachineState(domain::MachineStatus::Running);
    const auto policy = makeAccessPolicy({"manager"}, {domain::MachineStatus::Maintenance});

    const auto result = decision::DecisionEngine{}.evaluate(
        makeContext(event, zone, std::cref(employee), machineState, policy)
    );

    assert(result.type == domain::AccessDecisionType::Allowed);
    assert(result.reason == decision::DecisionReasons::InactiveZoneAccessRules);
    assert(!result.shouldCreateAlarm);
}

void allowsIdentifiedEmployeeInSafeZone() {
    const auto event = makeZoneEntryEvent();
    const auto zone = makeZone(domain::ZoneType::Safe);
    const auto employee = makeEmployee({"visitor"});
    const auto machineState = makeMachineState(domain::MachineStatus::Running);
    const auto policy = makeAccessPolicy({"maintenance"}, {domain::MachineStatus::Stopped});

    const auto result = decision::DecisionEngine{}.evaluate(
        makeContext(event, zone, std::cref(employee), machineState, policy)
    );

    assert(result.type == domain::AccessDecisionType::Allowed);
    assert(result.reason == decision::DecisionReasons::SafeZoneAccess);
    assert(!result.shouldCreateAlarm);
}

void createsViolationForInactiveEmployee() {
    const auto event = makeZoneEntryEvent();
    const auto zone = makeZone();
    const auto employee = makeEmployee({"maintenance"}, domain::EmployeeStatus::Inactive);
    const auto machineState = makeMachineState();
    const auto policy = makeAccessPolicy();

    const auto result = decision::DecisionEngine{}.evaluate(
        makeContext(event, zone, std::cref(employee), machineState, policy)
    );

    assert(result.type == domain::AccessDecisionType::Violation);
    assert(result.reason == decision::DecisionReasons::InactiveEmployee);
    assert(result.shouldCreateAlarm);
}

void createsViolationForDeniedMachineState() {
    const auto event = makeZoneEntryEvent();
    const auto zone = makeZone();
    const auto employee = makeEmployee();
    const auto machineState = makeMachineState(domain::MachineStatus::Running);
    const auto policy = makeAccessPolicy({"maintenance"}, {domain::MachineStatus::Stopped});

    const auto result = decision::DecisionEngine{}.evaluate(
        makeContext(event, zone, std::cref(employee), machineState, policy)
    );

    assert(result.type == domain::AccessDecisionType::Violation);
    assert(result.reason == decision::DecisionReasons::MachineStateDenied);
    assert(result.shouldCreateAlarm);
}

void createsViolationForDeniedRole() {
    const auto event = makeZoneEntryEvent();
    const auto zone = makeZone();
    const auto employee = makeEmployee({"visitor"});
    const auto machineState = makeMachineState();
    const auto policy = makeAccessPolicy({"maintenance"}, {domain::MachineStatus::Stopped});

    const auto result = decision::DecisionEngine{}.evaluate(
        makeContext(event, zone, std::cref(employee), machineState, policy)
    );

    assert(result.type == domain::AccessDecisionType::Violation);
    assert(result.reason == decision::DecisionReasons::RoleDenied);
    assert(result.shouldCreateAlarm);
}

void allowsEmployeeWithAllowedRoleAndMachineState() {
    const auto event = makeZoneEntryEvent();
    const auto zone = makeZone();
    const auto employee = makeEmployee({"maintenance"});
    const auto machineState = makeMachineState(domain::MachineStatus::Stopped);
    const auto policy = makeAccessPolicy({"maintenance"}, {domain::MachineStatus::Stopped});

    const auto result = decision::DecisionEngine{}.evaluate(
        makeContext(event, zone, std::cref(employee), machineState, policy)
    );

    assert(result.type == domain::AccessDecisionType::Allowed);
    assert(result.reason == decision::DecisionReasons::AccessAllowed);
    assert(!result.shouldCreateAlarm);
}

}

int main() {
    allowsWhenNoZoneEntryEventAndClearsExistingAlarm();
    returnsPendingIdentityWhileGracePeriodIsActive();
    createsUnknownIdentityAlarmAfterGracePeriodExpires();
    createsUnknownIdentityAlarmInInactiveZoneAfterGracePeriodExpires();
    allowsIdentifiedEmployeeInInactiveZone();
    allowsIdentifiedEmployeeInSafeZone();
    createsViolationForInactiveEmployee();
    createsViolationForDeniedMachineState();
    createsViolationForDeniedRole();
    allowsEmployeeWithAllowedRoleAndMachineState();
}
