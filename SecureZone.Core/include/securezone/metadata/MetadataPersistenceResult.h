#pragma once

#include <cstddef>

namespace securezone::metadata {

struct MetadataPersistenceResult {
    std::size_t tracksUpserted{};
    std::size_t eventsCreated{};
};

}
