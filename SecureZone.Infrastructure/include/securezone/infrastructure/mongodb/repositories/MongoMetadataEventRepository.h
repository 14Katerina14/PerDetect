#pragma once

#include <mongocxx/collection.hpp>

#include <cstdint>
#include <string>
#include <vector>

#include "securezone/domain/MetadataEvent.h"
#include "securezone/repository/IMetadataEventRepository.h"

namespace securezone::infrastructure::mongodb::repositories {

class MongoMetadataEventRepository final : public repository::IMetadataEventRepository {
public:
    explicit MongoMetadataEventRepository(mongocxx::collection metadataEventsCollection);

    void create(const domain::MetadataEvent& metadataEvent) override;

    std::vector<domain::MetadataEvent> findRecentByTrackId(
        const std::string& trackId,
        std::int64_t limit
    ) const override;

private:
    mongocxx::collection metadataEventsCollection_;
};

}
