#pragma once

#include <string>
namespace securezone::domain {
enum class AccessDecisionType { Allowed, Violation, UnknownIdentity, ZoneInactive, NotPerson };
struct AccessDecision { AccessDecisionType type{AccessDecisionType::Violation}; std::string reason; };
}
