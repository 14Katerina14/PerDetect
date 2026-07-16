#include "securezone/infrastructure/mongodb/repositories/MongoCameraObjectTrackRepository.h"
#include "securezone/infrastructure/mongodb/MongoDocumentMappers.h"

#include <bsoncxx/builder/basic/array.hpp>
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>
#include <mongocxx/options/find.hpp>
#include <mongocxx/options/update.hpp>

#include <utility>

namespace securezone::infrastructure::mongodb::repositories {
using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::make_document;

MongoCameraObjectTrackRepository::MongoCameraObjectTrackRepository(mongocxx::collection collection)
    : collection_{std::move(collection)} {
}

void MongoCameraObjectTrackRepository::upsertObservation(const domain::CameraObjectTrack& track) {
    mongocxx::options::update options;
    options.upsert(true);
    collection_.update_one(
        make_document(kvp("cameraId", track.cameraId), kvp("objectId", track.objectId)),
        make_document(
            kvp("$setOnInsert", make_document(
                kvp("cameraId", track.cameraId),
                kvp("objectId", track.objectId),
                kvp("objectType", track.objectType),
                kvp("firstSeenAt", bsoncxx::types::b_date{track.firstSeenAt})
            )),
            kvp("$set", make_document(
                kvp("lastSeenAt", bsoncxx::types::b_date{track.lastSeenAt}),
                kvp("status", "active")
            ))
        ),
        options
    );
}

std::optional<domain::CameraObjectTrack>
MongoCameraObjectTrackRepository::findByCameraAndObject(
    const std::string& cameraId,
    const std::string& objectId
) const {
    const auto document = collection_.find_one(
        make_document(kvp("cameraId", cameraId), kvp("objectId", objectId))
    );
    if (!document) return std::nullopt;
    return mapCameraObjectTrackDocument(document->view());
}

void MongoCameraObjectTrackRepository::markLost(
    const std::string& cameraId,
    const std::string& objectId,
    std::chrono::system_clock::time_point lostAt
) {
    collection_.update_one(
        make_document(kvp("cameraId", cameraId), kvp("objectId", objectId)),
        make_document(kvp("$set", make_document(
            kvp("lastSeenAt", bsoncxx::types::b_date{lostAt}),
            kvp("status", "lost")
        )))
    );
}

std::vector<domain::CameraObjectTrack> MongoCameraObjectTrackRepository::findRecentHumans(
    const std::string& cameraId,
    std::chrono::system_clock::time_point seenAfter,
    std::chrono::system_clock::time_point seenAtOrBefore
) const {
    bsoncxx::builder::basic::array types;
    types.append("Human");
    types.append("Person");
    mongocxx::options::find options;
    options.sort(make_document(kvp("firstSeenAt", -1)));
    options.limit(20);
    auto cursor = collection_.find(
        make_document(
            kvp("cameraId", cameraId),
            kvp("objectType", make_document(kvp("$in", types))),
            kvp("status", "active"),
            kvp("firstSeenAt", make_document(
                kvp("$gte", bsoncxx::types::b_date{seenAfter}),
                kvp("$lte", bsoncxx::types::b_date{seenAtOrBefore})
            ))
        ),
        options
    );
    std::vector<domain::CameraObjectTrack> tracks;
    for (const auto& document : cursor) tracks.push_back(mapCameraObjectTrackDocument(document));
    return tracks;
}

}
