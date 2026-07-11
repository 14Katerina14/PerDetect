#include "securezone/decision/DecisionEngine.h"

#include <algorithm>
#include <string>
#include <utility>

namespace securezone::decision {

namespace {

domain::AccessDecision makeDecision(
    domain::AccessDecisionType type,
    std::string reason,
    bool shouldCreateAlarm,
    bool shouldClearAlarm
) {
    return domain::AccessDecision{
        type,
        std::move(reason),
        shouldCreateAlarm,
        shouldClearAlarm
    };
}

bool hasAllowedMachineState(
    const domain::AccessPolicy& policy,
    domain::MachineStatus machineStatus
) {
    return std::find(
        policy.machineStatesAllowed.begin(),
        policy.machineStatesAllowed.end(),
        machineStatus
    ) != policy.machineStatesAllowed.end();
}

bool hasAllowedRole(
    const domain::AccessPolicy& policy,
    const domain::Employee& employee
) {
    return std::any_of(
        employee.roles.begin(),
        employee.roles.end(),
        [&](const std::string& role) {
            return std::find(
                policy.allowedRoles.begin(),
                policy.allowedRoles.end(),
                role
            ) != policy.allowedRoles.end();
        }
    );
}

}

domain::AccessDecision DecisionEngine::evaluate(const DecisionContext& context) const {
    if (context.detection.objectClass != domain::ObjectClass::Person) {
        return makeDecision(
            domain::AccessDecisionType::Ignored,
            "Detected object is not a person.",
            false,
            false
        );
    }

    if (!context.isInsideZone) {
        return makeDecision(
            domain::AccessDecisionType::Allowed,
            "Person is outside the zone.",
            false,
            context.hadActiveAlarm
        );
    }

    if (!context.employee.has_value()) {
        if (context.isIdentityGracePeriodActive) {
            return makeDecision(
                domain::AccessDecisionType::PendingIdentity,
                "Waiting for identity association.",
                false,
                false
            );
        }

        return makeDecision(
            domain::AccessDecisionType::UnknownIdentity,
            "Person identity is unknown.",
            true,
            false
        );
    }

    const auto& employee = context.employee->get();

    if (employee.status != domain::EmployeeStatus::Active) {
        return makeDecision(
            domain::AccessDecisionType::Violation,
            "Employee is inactive.",
            true,
            false
        );
    }

    if (context.zone.status != domain::ZoneStatus::Active) {
        return makeDecision(
            domain::AccessDecisionType::Allowed,
            "Zone is inactive for access rules.",
            false,
            false
        );
    }

    if (context.zone.type == domain::ZoneType::Safe) {
        return makeDecision(
            domain::AccessDecisionType::Allowed,
            "Identified employee is inside a safe zone.",
            false,
            false
        );
    }

    if (!hasAllowedMachineState(context.accessPolicy, context.machineState.status)) {
        return makeDecision(
            domain::AccessDecisionType::Violation,
            "Machine state does not allow access.",
            true,
            false
        );
    }

    if (!hasAllowedRole(context.accessPolicy, employee)) {
        return makeDecision(
            domain::AccessDecisionType::Violation,
            "Employee role is not allowed in this zone.",
            true,
            false
        );
    }

    return makeDecision(
        domain::AccessDecisionType::Allowed,
        "Employee is allowed in this zone.",
        false,
        false
    );
}

}
