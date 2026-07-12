#include "securezone/infrastructure/mongodb/repositories/MongoZoneRepository.h"

#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>

#include <utility>

#include "securezone/infrastructure/mongodb/MongoDocumentMappers.h"

namespace securezone::infrastructure::mongodb::repositories {

namespace {

constexpr const char* ZoneIdField = "zoneId";
constexpr const char* StatusField = "status";
constexpr const char* ActiveStatus = "active";

}

MongoZoneRepository::MongoZoneRepository(
    mongocxx::collection zonesCollection
) : zonesCollection_{std::move(zonesCollection)} {
}

std::optional<domain::Zone> MongoZoneRepository::findByZoneId(
    const std::string& zoneId
) const {
    bsoncxx::builder::basic::document filter;
    filter.append(bsoncxx::builder::basic::kvp(ZoneIdField, zoneId));

    auto result = zonesCollection_.find_one(filter.view());
    if (!result) {
        return std::nullopt;
    }

    return mapZoneDocument(result->view());
}

std::optional<domain::Zone> MongoZoneRepository::findActiveByZoneId(
    const std::string& zoneId
) const {
    bsoncxx::builder::basic::document filter;
    filter.append(
        bsoncxx::builder::basic::kvp(ZoneIdField, zoneId),
        bsoncxx::builder::basic::kvp(StatusField, ActiveStatus)
    );

    auto result = zonesCollection_.find_one(filter.view());
    if (!result) {
        return std::nullopt;
    }

    return mapZoneDocument(result->view());
}

}
