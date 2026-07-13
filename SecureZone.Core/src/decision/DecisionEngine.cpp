#include "securezone/decision/DecisionEngine.h"
namespace securezone::decision {
domain::AccessDecision DecisionEngine::evaluate(const domain::Detection& detection, const domain::Zone& zone, const std::optional<domain::Employee>& employee, const domain::MachineState& machineState, const domain::AccessPolicy& policy, bool isInsideZone) const {
    if (detection.objectClass != domain::ObjectClass::Person) return {domain::AccessDecisionType::NotPerson, "Detected object is not a person."};
    if (zone.status != domain::ZoneStatus::Active) return {domain::AccessDecisionType::ZoneInactive, "Zone is inactive."};
    if (!isInsideZone) return {domain::AccessDecisionType::Allowed, "Person is outside the zone."};
    return AccessPolicyEvaluator{}.evaluate(employee, machineState, policy);
}
}
