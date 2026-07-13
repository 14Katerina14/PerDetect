#pragma once

#include <optional>
#include "securezone/domain/AccessDecision.h"
#include "securezone/domain/AccessPolicy.h"
#include "securezone/domain/Employee.h"
#include "securezone/domain/MachineState.h"
namespace securezone::decision {
class AccessPolicyEvaluator { public: domain::AccessDecision evaluate(const std::optional<domain::Employee>& employee, const domain::MachineState& machineState, const domain::AccessPolicy& policy) const; };
}
