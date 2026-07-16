#pragma once

#include <mongocxx/collection.hpp>

#include <optional>
#include <string>

#include "securezone/domain/Zone.h"
#include "securezone/repository/IZoneRepository.h"
#include "securezone/repository/IZoneStatusReadRepository.h"

namespace securezone::infrastructure::mongodb::repositories {

class MongoZoneRepository final
    : public repository::IZoneRepository,
      public repository::IZoneStatusReadRepository {
public:
    explicit MongoZoneRepository(mongocxx::collection zonesCollection);

    std::optional<domain::Zone> findByZoneId(
        const std::string& zoneId
    ) const override;

    std::optional<domain::Zone> findActiveByZoneId(
        const std::string& zoneId
    ) const override;

    std::optional<domain::Zone> findActiveByXProtectEventName(
        const std::string& xprotectEventName
    ) const override;

    bool save(const domain::Zone& zone) override;

    std::vector<domain::Zone> findAll(
        std::size_t limit,
        const std::optional<std::string>& cameraId = std::nullopt
    ) const override;

private:
    mutable mongocxx::collection zonesCollection_;
};

}
