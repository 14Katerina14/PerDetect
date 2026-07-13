#include "securezone/infrastructure/mongodb/repositories/MongoCameraTrackRepository.h"

#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>
#include <bsoncxx/types.hpp>
#include <mongocxx/options/update.hpp>

#include <chrono>
#include <utility>

#include "securezone/infrastructure/mongodb/MongoDocumentMappers.h"

namespace securezone::infrastructure::mongodb::repositories {

namespace {

using Clock = std::chrono::system_clock;

constexpr const char* LostStatus = "lost";
constexpr const char* StatusField = "status";
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

bsoncxx::document::value toCameraTrackDocument(const domain::CameraTrack& cameraTrack) {
    bsoncxx::builder::basic::document document;
    auto bbox = toBboxDocument(cameraTrack.bbox);

    document.append(
        bsoncxx::builder::basic::kvp(TrackIdField, cameraTrack.trackId),
        bsoncxx::builder::basic::kvp("cameraId", cameraTrack.cameraId),
        bsoncxx::builder::basic::kvp("firstSeenAt", toBsonDate(cameraTrack.firstSeenAt)),
        bsoncxx::builder::basic::kvp("lastSeenAt", toBsonDate(cameraTrack.lastSeenAt)),
        bsoncxx::builder::basic::kvp("currentZoneId", cameraTrack.currentZoneId),
        bsoncxx::builder::basic::kvp("objectClass", cameraTrack.objectClass),
        bsoncxx::builder::basic::kvp("bbox", bbox.view()),
        bsoncxx::builder::basic::kvp(StatusField, cameraTrack.status)
    );

    return document.extract();
}

}

MongoCameraTrackRepository::MongoCameraTrackRepository(
    mongocxx::collection cameraTracksCollection
) : cameraTracksCollection_{std::move(cameraTracksCollection)} {
}

std::optional<domain::CameraTrack> MongoCameraTrackRepository::findByTrackId(
    const std::string& trackId
) const {
    bsoncxx::builder::basic::document filter;
    filter.append(bsoncxx::builder::basic::kvp(TrackIdField, trackId));

    auto result = cameraTracksCollection_.find_one(filter.view());
    if (!result) {
        return std::nullopt;
    }

    return mapCameraTrackDocument(result->view());
}

void MongoCameraTrackRepository::upsert(const domain::CameraTrack& cameraTrack) {
    bsoncxx::builder::basic::document filter;
    filter.append(bsoncxx::builder::basic::kvp(TrackIdField, cameraTrack.trackId));

    auto document = toCameraTrackDocument(cameraTrack);
    bsoncxx::builder::basic::document update;
    update.append(bsoncxx::builder::basic::kvp("$set", document.view()));

    mongocxx::options::update options;
    options.upsert(true);

    cameraTracksCollection_.update_one(filter.view(), update.view(), options);
}

void MongoCameraTrackRepository::markLost(
    const std::string& trackId,
    Clock::time_point lostAt
) {
    bsoncxx::builder::basic::document filter;
    filter.append(bsoncxx::builder::basic::kvp(TrackIdField, trackId));

    bsoncxx::builder::basic::document updateFields;
    updateFields.append(
        bsoncxx::builder::basic::kvp(StatusField, LostStatus),
        bsoncxx::builder::basic::kvp("lastSeenAt", toBsonDate(lostAt))
    );

    bsoncxx::builder::basic::document update;
    update.append(bsoncxx::builder::basic::kvp("$set", updateFields.extract()));

    cameraTracksCollection_.update_one(filter.view(), update.view());
}

}
