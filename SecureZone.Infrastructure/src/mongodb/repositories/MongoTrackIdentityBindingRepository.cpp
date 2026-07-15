#include "securezone/infrastructure/mongodb/repositories/MongoTrackIdentityBindingRepository.h"
#include "securezone/infrastructure/mongodb/MongoDocumentMappers.h"

#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>
#include <mongocxx/options/find.hpp>

#include <utility>

namespace securezone::infrastructure::mongodb::repositories {
using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::make_document;

MongoTrackIdentityBindingRepository::MongoTrackIdentityBindingRepository(mongocxx::collection collection)
    : collection_{std::move(collection)} {
}

std::optional<domain::TrackIdentityBinding> MongoTrackIdentityBindingRepository::findActiveByTrack(
    const std::string& cameraId,
    const std::string& objectId,
    std::chrono::system_clock::time_point at
) const {
    mongocxx::options::find options;
    options.sort(make_document(kvp("boundAt", -1)));
    const auto document = collection_.find_one(
        make_document(
            kvp("cameraId", cameraId),
            kvp("objectId", objectId),
            kvp("status", "active"),
            kvp("boundAt", make_document(kvp("$lte", bsoncxx::types::b_date{at}))),
            kvp("expiresAt", make_document(kvp("$gte", bsoncxx::types::b_date{at})))
        ),
        options
    );
    if (!document) return std::nullopt;
    return mapTrackIdentityBindingDocument(document->view());
}

void MongoTrackIdentityBindingRepository::create(const domain::TrackIdentityBinding& binding) {
    collection_.insert_one(make_document(
        kvp("bindingId", binding.bindingId),
        kvp("cameraId", binding.cameraId),
        kvp("objectId", binding.objectId),
        kvp("employeeId", binding.employeeId),
        kvp("sourceCheckinId", binding.sourceCheckinId),
        kvp("presenceSessionId", binding.presenceSessionId),
        kvp("boundAt", bsoncxx::types::b_date{binding.boundAt}),
        kvp("expiresAt", bsoncxx::types::b_date{binding.expiresAt}),
        kvp("status", "active")
    ));
}

}
