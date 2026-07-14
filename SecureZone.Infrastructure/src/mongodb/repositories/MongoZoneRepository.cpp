#include "securezone/infrastructure/mongodb/repositories/MongoZoneRepository.h"

#include <bsoncxx/builder/basic/array.hpp>
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>
#include <mongocxx/options/update.hpp>

#include <utility>

#include "securezone/infrastructure/mongodb/MongoDocumentMappers.h"

namespace securezone::infrastructure::mongodb::repositories {

namespace {

constexpr const char* ZoneIdField = "zoneId";
constexpr const char* StatusField = "status";
constexpr const char* ActiveStatus = "active";

const char* zoneTypeToString(domain::ZoneType type) {
    switch (type) {
        case domain::ZoneType::Safe:
            return "safe";
        case domain::ZoneType::Restricted:
            return "restricted";
        case domain::ZoneType::Dangerous:
            return "dangerous";
    }

    return "restricted";
}

const char* zoneStatusToString(domain::ZoneStatus status) {
    return status == domain::ZoneStatus::Active ? "active" : "inactive";
}

bsoncxx::array::value toPolygonArray(const std::vector<domain::Point>& polygon) {
    bsoncxx::builder::basic::array array;
    for (const auto& point : polygon) {
        bsoncxx::builder::basic::document pointDocument;
        pointDocument.append(
            bsoncxx::builder::basic::kvp("x", point.x),
            bsoncxx::builder::basic::kvp("y", point.y)
        );
        array.append(pointDocument.extract());
    }
    return array.extract();
}

bsoncxx::document::value toZoneDocument(const domain::Zone& zone) {
    auto polygon = toPolygonArray(zone.polygon);
    bsoncxx::builder::basic::document document;
    document.append(
        bsoncxx::builder::basic::kvp(ZoneIdField, zone.zoneId),
        bsoncxx::builder::basic::kvp("name", zone.name),
        bsoncxx::builder::basic::kvp("cameraId", zone.cameraId),
        bsoncxx::builder::basic::kvp("type", zoneTypeToString(zone.type)),
        bsoncxx::builder::basic::kvp("polygon", polygon.view()),
        bsoncxx::builder::basic::kvp(StatusField, zoneStatusToString(zone.status)),
        bsoncxx::builder::basic::kvp("relatedMachineId", zone.relatedMachineId)
    );
    return document.extract();
}

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

bool MongoZoneRepository::save(const domain::Zone& zone) {
    bsoncxx::builder::basic::document filter;
    filter.append(bsoncxx::builder::basic::kvp(ZoneIdField, zone.zoneId));

    auto zoneDocument = toZoneDocument(zone);
    bsoncxx::builder::basic::document update;
    update.append(bsoncxx::builder::basic::kvp("$set", zoneDocument.view()));

    mongocxx::options::update options;
    options.upsert(true);
    return zonesCollection_.update_one(filter.view(), update.view(), options).has_value();
}

}
