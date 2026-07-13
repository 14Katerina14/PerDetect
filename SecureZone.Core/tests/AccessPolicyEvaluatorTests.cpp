#include <cassert>
#include "securezone/decision/AccessPolicyEvaluator.h"
int main() {
    using namespace securezone::domain; using securezone::decision::AccessPolicyEvaluator;
    const AccessPolicy policy{"p", "z", {"operator"}, {MachineStatus::Stopped}}; const MachineState stopped{"m", MachineStatus::Stopped};
    const Employee allowed{"e", "Name", {"operator"}}; const Employee denied{"e", "Name", {"guest"}};
    AccessPolicyEvaluator evaluator;
    assert(evaluator.evaluate(allowed, stopped, policy).type == AccessDecisionType::Allowed);
    assert(evaluator.evaluate(denied, stopped, policy).type == AccessDecisionType::Violation);
    assert(evaluator.evaluate(allowed, {"m", MachineStatus::Running}, policy).type == AccessDecisionType::Violation);
    assert(evaluator.evaluate(std::nullopt, stopped, policy).type == AccessDecisionType::UnknownIdentity);
}
