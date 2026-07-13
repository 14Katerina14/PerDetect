#pragma once

#include <cstddef>

namespace securezone::metadata {

struct MetadataDecisionTriggerResult {
    std::size_t detectionsChecked{};
    std::size_t decisionsEvaluated{};
    std::size_t allowed{};
    std::size_t pendingIdentity{};
    std::size_t violations{};
    std::size_t ignored{};
    std::size_t alarmsCreated{};
    std::size_t alarmsResolved{};
};

}
