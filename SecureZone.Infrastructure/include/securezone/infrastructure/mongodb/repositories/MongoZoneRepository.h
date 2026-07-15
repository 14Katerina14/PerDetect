#pragma once

#include <mongocxx/collection.hpp>

#include <optional>
#include <string>

#include "securezone/domain/Zone.h"
#include "securezone/repository/IZoneRepository.h"

namespace securezone::infrastructure::mongodb::repositories {

class MongoZoneRepository final : public repository::IZoneRepository {
public:
    explicit MongoZoneRepository(mongocxx::collection zonesCollection);

    std::optional<domain::Zone> findByZoneId(
        const std::string& zoneId
    ) const override;

    std::optional<domain::Zone> findActiveByZoneId(
        const std::string& zoneId
    ) const override;

    bool save(const domain::Zone& zone) override;

private:
    mutable mongocxx::collection zonesCollection_;
};

}
