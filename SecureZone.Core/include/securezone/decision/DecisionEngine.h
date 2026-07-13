#pragma once

#include <optional>
#include "securezone/decision/AccessPolicyEvaluator.h"
#include "securezone/domain/Detection.h"
#include "securezone/domain/Zone.h"
namespace securezone::decision { class DecisionEngine { public: domain::AccessDecision evaluate(const domain::Detection&, const domain::Zone&, const std::optional<domain::Employee>&, const domain::MachineState&, const domain::AccessPolicy&, bool isInsideZone) const; }; }
