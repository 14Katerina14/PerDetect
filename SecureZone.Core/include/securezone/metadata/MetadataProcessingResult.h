#pragma once

#include <cstddef>

namespace securezone::metadata {

struct MetadataProcessingResult {
    std::size_t detectionsProcessed{};
    std::size_t tracksUpserted{};
    std::size_t eventsCreated{};
};

}
