#include "securezone/decision/DecisionEngine.h"

#include <cassert>
#include <chrono>
#include <functional>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace {

using namespace securezone;

domain::Detection makeDetection(domain::ObjectClass objectClass = domain::ObjectClass::Person) {
    domain::Detection detection{};
    detection.trackId = "track-1";
    detection.cameraId = "camera-1";
    detection.objectClass = objectClass;
    detection.confidence = 0.95;
    detection.timestamp = std::chrono::system_clock::now();
    return detection;
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
    employee.position = "Maintenance Technician";
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
    const domain::Detection& detection,
    const domain::Zone& zone,
    const std::optional<std::reference_wrapper<const domain::Employee>>& employee,
    const domain::MachineState& machineState,
    const domain::AccessPolicy& accessPolicy
) {
    decision::DecisionContext context{
        detection,
        zone,
        employee,
        machineState,
        accessPolicy
    };
    context.isInsideZone = true;
    return context;
}

void ignoresNonPersonDetections() {
    const auto detection = makeDetection(domain::ObjectClass::Vehicle);
    const auto zone = makeZone();
    const auto employee = makeEmployee();
    const auto machineState = makeMachineState();
    const auto policy = makeAccessPolicy();

    const auto result = decision::DecisionEngine{}.evaluate(
        makeContext(detection, zone, std::cref(employee), machineState, policy)
    );

    assert(result.type == domain::AccessDecisionType::Ignored);
    assert(!result.shouldCreateAlarm);
}

void allowsPersonOutsideZoneAndClearsExistingAlarm() {
    const auto detection = makeDetection();
    const auto zone = makeZone();
    const auto employee = makeEmployee();
    const auto machineState = makeMachineState();
    const auto policy = makeAccessPolicy();
    auto context = makeContext(detection, zone, std::cref(employee), machineState, policy);
    context.isInsideZone = false;
    context.hadActiveAlarm = true;

    const auto result = decision::DecisionEngine{}.evaluate(context);

    assert(result.type == domain::AccessDecisionType::Allowed);
    assert(!result.shouldCreateAlarm);
    assert(result.shouldClearAlarm);
}

void returnsPendingIdentityWhileGracePeriodIsActive() {
    const auto detection = makeDetection();
    const auto zone = makeZone();
    const auto machineState = makeMachineState();
    const auto policy = makeAccessPolicy();
    auto context = makeContext(detection, zone, std::nullopt, machineState, policy);
    context.isIdentityGracePeriodActive = true;

    const auto result = decision::DecisionEngine{}.evaluate(context);

    assert(result.type == domain::AccessDecisionType::PendingIdentity);
    assert(!result.shouldCreateAlarm);
}

void createsUnknownIdentityAlarmAfterGracePeriodExpires() {
    const auto detection = makeDetection();
    const auto zone = makeZone(domain::ZoneType::Safe);
    const auto machineState = makeMachineState();
    const auto policy = makeAccessPolicy();
    const auto context = makeContext(detection, zone, std::nullopt, machineState, policy);

    const auto result = decision::DecisionEngine{}.evaluate(context);

    assert(result.type == domain::AccessDecisionType::UnknownIdentity);
    assert(result.shouldCreateAlarm);
}

void createsUnknownIdentityAlarmInInactiveZoneAfterGracePeriodExpires() {
    const auto detection = makeDetection();
    const auto zone = makeZone(domain::ZoneType::Safe, domain::ZoneStatus::Inactive);
    const auto machineState = makeMachineState();
    const auto policy = makeAccessPolicy();
    const auto context = makeContext(detection, zone, std::nullopt, machineState, policy);

    const auto result = decision::DecisionEngine{}.evaluate(context);

    assert(result.type == domain::AccessDecisionType::UnknownIdentity);
    assert(result.shouldCreateAlarm);
}

void allowsIdentifiedEmployeeInInactiveZone() {
    const auto detection = makeDetection();
    const auto zone = makeZone(domain::ZoneType::Dangerous, domain::ZoneStatus::Inactive);
    const auto employee = makeEmployee();
    const auto machineState = makeMachineState(domain::MachineStatus::Running);
    const auto policy = makeAccessPolicy({"manager"}, {domain::MachineStatus::Maintenance});

    const auto result = decision::DecisionEngine{}.evaluate(
        makeContext(detection, zone, std::cref(employee), machineState, policy)
    );

    assert(result.type == domain::AccessDecisionType::Allowed);
    assert(!result.shouldCreateAlarm);
}

void allowsIdentifiedEmployeeInSafeZone() {
    const auto detection = makeDetection();
    const auto zone = makeZone(domain::ZoneType::Safe);
    const auto employee = makeEmployee({"visitor"});
    const auto machineState = makeMachineState(domain::MachineStatus::Running);
    const auto policy = makeAccessPolicy({"maintenance"}, {domain::MachineStatus::Stopped});

    const auto result = decision::DecisionEngine{}.evaluate(
        makeContext(detection, zone, std::cref(employee), machineState, policy)
    );

    assert(result.type == domain::AccessDecisionType::Allowed);
    assert(!result.shouldCreateAlarm);
}

void createsViolationForInactiveEmployee() {
    const auto detection = makeDetection();
    const auto zone = makeZone();
    const auto employee = makeEmployee({"maintenance"}, domain::EmployeeStatus::Inactive);
    const auto machineState = makeMachineState();
    const auto policy = makeAccessPolicy();

    const auto result = decision::DecisionEngine{}.evaluate(
        makeContext(detection, zone, std::cref(employee), machineState, policy)
    );

    assert(result.type == domain::AccessDecisionType::Violation);
    assert(result.shouldCreateAlarm);
}

void createsViolationForDeniedMachineState() {
    const auto detection = makeDetection();
    const auto zone = makeZone();
    const auto employee = makeEmployee();
    const auto machineState = makeMachineState(domain::MachineStatus::Running);
    const auto policy = makeAccessPolicy({"maintenance"}, {domain::MachineStatus::Stopped});

    const auto result = decision::DecisionEngine{}.evaluate(
        makeContext(detection, zone, std::cref(employee), machineState, policy)
    );

    assert(result.type == domain::AccessDecisionType::Violation);
    assert(result.shouldCreateAlarm);
}

void createsViolationForDeniedRole() {
    const auto detection = makeDetection();
    const auto zone = makeZone();
    const auto employee = makeEmployee({"visitor"});
    const auto machineState = makeMachineState();
    const auto policy = makeAccessPolicy({"maintenance"}, {domain::MachineStatus::Stopped});

    const auto result = decision::DecisionEngine{}.evaluate(
        makeContext(detection, zone, std::cref(employee), machineState, policy)
    );

    assert(result.type == domain::AccessDecisionType::Violation);
    assert(result.shouldCreateAlarm);
}

void allowsEmployeeWithAllowedRoleAndMachineState() {
    const auto detection = makeDetection();
    const auto zone = makeZone();
    const auto employee = makeEmployee({"maintenance"});
    const auto machineState = makeMachineState(domain::MachineStatus::Stopped);
    const auto policy = makeAccessPolicy({"maintenance"}, {domain::MachineStatus::Stopped});

    const auto result = decision::DecisionEngine{}.evaluate(
        makeContext(detection, zone, std::cref(employee), machineState, policy)
    );

    assert(result.type == domain::AccessDecisionType::Allowed);
    assert(!result.shouldCreateAlarm);
}

}

int main() {
    ignoresNonPersonDetections();
    allowsPersonOutsideZoneAndClearsExistingAlarm();
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
