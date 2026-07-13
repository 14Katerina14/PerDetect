#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "securezone/domain/MetadataEvent.h"

namespace securezone::repository {

class IMetadataEventRepository {
public:
    virtual ~IMetadataEventRepository() = default;

    virtual void create(const domain::MetadataEvent& metadataEvent) = 0;

    virtual std::vector<domain::MetadataEvent> findRecentByTrackId(
        const std::string& trackId,
        std::int64_t limit
    ) const = 0;
};

}
