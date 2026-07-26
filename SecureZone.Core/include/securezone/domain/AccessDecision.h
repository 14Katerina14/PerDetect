#pragma once

#include <string>
namespace securezone::domain {
enum class AccessDecisionType {
    Allowed,
    Violation,
    UnknownIdentity,
    PendingIdentity,
    Ignored
};

struct AccessDecision {
    AccessDecisionType type{AccessDecisionType::Violation};
    std::string reason;
    bool shouldCreateAlarm{};
    bool shouldClearAlarm{};
};
}
