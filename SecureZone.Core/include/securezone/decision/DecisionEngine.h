#pragma once

#include "securezone/decision/DecisionContext.h"
#include "securezone/domain/AccessDecision.h"

namespace securezone::decision {

class DecisionEngine {
public:
    domain::AccessDecision evaluate(const DecisionContext& context) const;
};

}
