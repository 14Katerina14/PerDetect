#include "securezone/decision/DecisionEngine.h"

namespace securezone::decision {

domain::AccessDecision DecisionEngine::evaluate(const DecisionContext& context) const {
    static_cast<void>(context);

    return domain::AccessDecision{};
}

}
