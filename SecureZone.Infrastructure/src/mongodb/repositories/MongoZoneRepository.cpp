#include "securezone/infrastructure/mongodb/repositories/MongoZoneRepository.h"

#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>
#include <mongocxx/options/update.hpp>

#include <utility>
#include <cstdint>
#include <exception>
#include <vector>
#include <mongocxx/options/find.hpp>

#include "securezone/infrastructure/mongodb/MongoDocumentMappers.h"

namespace securezone::infrastructure::mongodb::repositories {

namespace {

constexpr char ZoneIdField[] = "zoneId";
constexpr char StatusField[] = "status";
constexpr char ActiveStatus[] = "active";
constexpr char XProtectEventNameField[] = "xprotectEventName";

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

bsoncxx::document::value toZoneDocument(const domain::Zone& zone) {
    bsoncxx::builder::basic::document document;
    document.append(
        bsoncxx::builder::basic::kvp(ZoneIdField, zone.zoneId),
        bsoncxx::builder::basic::kvp("name", zone.name),
        bsoncxx::builder::basic::kvp("cameraId", zone.cameraId),
        bsoncxx::builder::basic::kvp("type", zoneTypeToString(zone.type)),
        bsoncxx::builder::basic::kvp(StatusField, zoneStatusToString(zone.status)),
        bsoncxx::builder::basic::kvp("relatedMachineId", zone.relatedMachineId),
        bsoncxx::builder::basic::kvp("xprotectEventName", zone.xprotectEventName)
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

std::optional<domain::Zone> MongoZoneRepository::findActiveByXProtectEventName(
    const std::string& xprotectEventName
) const {
    bsoncxx::builder::basic::document filter;
    filter.append(
        bsoncxx::builder::basic::kvp(XProtectEventNameField, xprotectEventName),
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

std::vector<domain::Zone> MongoZoneRepository::findAll(
    std::size_t limit,
    const std::optional<std::string>& cameraId
) const {
    bsoncxx::builder::basic::document filter;
    if (cameraId.has_value() && !cameraId->empty()) {
        filter.append(bsoncxx::builder::basic::kvp("cameraId", *cameraId));
    }

    mongocxx::options::find options;
    options.sort(bsoncxx::builder::basic::make_document(
        bsoncxx::builder::basic::kvp("name", 1)
    ));
    options.limit(static_cast<std::int64_t>(limit));

    std::vector<domain::Zone> zones;
    for (const auto& document : zonesCollection_.find(filter.view(), options)) {
        try {
            zones.push_back(mapZoneDocument(document));
        } catch (const std::exception&) {
        }
    }
    return zones;
}

}
