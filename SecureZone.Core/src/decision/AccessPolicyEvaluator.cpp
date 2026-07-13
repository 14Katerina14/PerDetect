#include "securezone/decision/AccessPolicyEvaluator.h"
#include <algorithm>
namespace securezone::decision {
domain::AccessDecision AccessPolicyEvaluator::evaluate(const std::optional<domain::Employee>& employee, const domain::MachineState& machineState, const domain::AccessPolicy& policy) const {
    if (!employee) return {domain::AccessDecisionType::UnknownIdentity, "Person identity is unknown."};
    if (employee->status != domain::EmployeeStatus::Active) return {domain::AccessDecisionType::Violation, "Employee is inactive."};
    const auto machineAllowed = std::find(policy.machineStatesAllowed.begin(), policy.machineStatesAllowed.end(), machineState.status) != policy.machineStatesAllowed.end();
    if (!machineAllowed) return {domain::AccessDecisionType::Violation, "Machine state does not allow access."};
    const auto roleAllowed = std::any_of(employee->roles.begin(), employee->roles.end(), [&](const std::string& role) { return std::find(policy.allowedRoles.begin(), policy.allowedRoles.end(), role) != policy.allowedRoles.end(); });
    return roleAllowed ? domain::AccessDecision{domain::AccessDecisionType::Allowed, "Employee is allowed in this zone."} : domain::AccessDecision{domain::AccessDecisionType::Violation, "Employee role is not allowed in this zone."};
}
}
