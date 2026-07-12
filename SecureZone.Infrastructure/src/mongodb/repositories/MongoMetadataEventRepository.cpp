#include "securezone/infrastructure/mongodb/repositories/MongoMetadataEventRepository.h"

#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>
#include <bsoncxx/types.hpp>
#include <mongocxx/options/find.hpp>

#include <chrono>
#include <utility>

#include "securezone/infrastructure/mongodb/MongoDocumentMappers.h"

namespace securezone::infrastructure::mongodb::repositories {

namespace {

using Clock = std::chrono::system_clock;

constexpr const char* TrackIdField = "trackId";

bsoncxx::types::b_date toBsonDate(Clock::time_point timePoint) {
    return bsoncxx::types::b_date{
        std::chrono::duration_cast<std::chrono::milliseconds>(
            timePoint.time_since_epoch()
        )
    };
}

bsoncxx::document::value toBboxDocument(const domain::BoundingBox& bbox) {
    bsoncxx::builder::basic::document document;
    document.append(
        bsoncxx::builder::basic::kvp("x", bbox.x),
        bsoncxx::builder::basic::kvp("y", bbox.y),
        bsoncxx::builder::basic::kvp("width", bbox.width),
        bsoncxx::builder::basic::kvp("height", bbox.height)
    );
    return document.extract();
}

bsoncxx::document::value toMetadataEventDocument(const domain::MetadataEvent& metadataEvent) {
    bsoncxx::builder::basic::document document;
    auto bbox = toBboxDocument(metadataEvent.bbox);

    document.append(
        bsoncxx::builder::basic::kvp("eventId", metadataEvent.eventId),
        bsoncxx::builder::basic::kvp("cameraId", metadataEvent.cameraId),
        bsoncxx::builder::basic::kvp(TrackIdField, metadataEvent.trackId),
        bsoncxx::builder::basic::kvp("timestamp", toBsonDate(metadataEvent.timestamp)),
        bsoncxx::builder::basic::kvp("objectClass", metadataEvent.objectClass),
        bsoncxx::builder::basic::kvp("bbox", bbox.view()),
        bsoncxx::builder::basic::kvp("zoneId", metadataEvent.zoneId),
        bsoncxx::builder::basic::kvp("eventType", metadataEvent.eventType)
    );

    return document.extract();
}

}

MongoMetadataEventRepository::MongoMetadataEventRepository(
    mongocxx::collection metadataEventsCollection
) : metadataEventsCollection_{std::move(metadataEventsCollection)} {
}

void MongoMetadataEventRepository::create(const domain::MetadataEvent& metadataEvent) {
    auto document = toMetadataEventDocument(metadataEvent);
    metadataEventsCollection_.insert_one(document.view());
}

std::vector<domain::MetadataEvent> MongoMetadataEventRepository::findRecentByTrackId(
    const std::string& trackId,
    std::int64_t limit
) const {
    bsoncxx::builder::basic::document filter;
    filter.append(bsoncxx::builder::basic::kvp(TrackIdField, trackId));

    bsoncxx::builder::basic::document sort;
    sort.append(bsoncxx::builder::basic::kvp("timestamp", -1));

    mongocxx::options::find options;
    options.sort(sort.view());
    options.limit(limit);

    std::vector<domain::MetadataEvent> events;
    auto cursor = metadataEventsCollection_.find(filter.view(), options);
    for (const auto& document : cursor) {
        events.push_back(mapMetadataEventDocument(document));
    }

    return events;
}

}
