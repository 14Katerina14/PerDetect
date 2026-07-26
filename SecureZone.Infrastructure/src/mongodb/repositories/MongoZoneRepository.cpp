#include "securezone/infrastructure/mongodb/repositories/MongoZoneRepository.h"

#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>
#include <mongocxx/options/update.hpp>

#include <utility>
#include <cctype>
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
constexpr char IntegerPlaceholder[] = "<int>";

bool matchesXProtectEventPattern(
    const std::string& pattern,
    const std::string& eventName
) {
    std::size_t patternOffset = 0;
    std::size_t eventOffset = 0;

    while (true) {
        const auto placeholder = pattern.find(IntegerPlaceholder, patternOffset);
        const auto prefixLength = placeholder == std::string::npos
            ? pattern.size() - patternOffset
            : placeholder - patternOffset;

        if (eventName.compare(eventOffset, prefixLength, pattern, patternOffset, prefixLength) != 0) {
            return false;
        }
        eventOffset += prefixLength;

        if (placeholder == std::string::npos) {
            return eventOffset == eventName.size();
        }

        const auto digitsBegin = eventOffset;
        while (eventOffset < eventName.size()
            && std::isdigit(static_cast<unsigned char>(eventName[eventOffset]))) {
            ++eventOffset;
        }
        if (eventOffset == digitsBegin) {
            return false;
        }

        patternOffset = placeholder + sizeof(IntegerPlaceholder) - 1;
    }
}

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
    if (result) {
        return mapZoneDocument(result->view());
    }

    auto activeZones = zonesCollection_.find(
        bsoncxx::builder::basic::make_document(
            bsoncxx::builder::basic::kvp(StatusField, ActiveStatus)
        )
    );
    for (const auto& document : activeZones) {
        try {
            auto zone = mapZoneDocument(document);
            if (!zone.xprotectEventName.empty()
                && matchesXProtectEventPattern(zone.xprotectEventName, xprotectEventName)) {
                return zone;
            }
        } catch (const std::exception&) {
        }
    }

    return std::nullopt;
}

std::optional<domain::Zone> MongoZoneRepository::findActiveSafeByCameraId(
    const std::string& cameraId
) const {
    auto result = zonesCollection_.find_one(
        bsoncxx::builder::basic::make_document(
            bsoncxx::builder::basic::kvp("cameraId", cameraId),
            bsoncxx::builder::basic::kvp("type", "safe"),
            bsoncxx::builder::basic::kvp(StatusField, ActiveStatus)
        )
    );
    if (!result) return std::nullopt;
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
