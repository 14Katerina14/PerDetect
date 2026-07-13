#include "securezone/infrastructure/mongodb/repositories/MongoTrackIdentityBindingRepository.h"

#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>
#include <bsoncxx/types.hpp>

#include <chrono>
#include <utility>

#include "securezone/infrastructure/mongodb/MongoDocumentMappers.h"

namespace securezone::infrastructure::mongodb::repositories {

namespace {

using Clock = std::chrono::system_clock;

constexpr const char* BindingIdField = "bindingId";
constexpr const char* ConfirmedStatus = "confirmed";
constexpr const char* StatusField = "status";
constexpr const char* TrackIdField = "trackId";

bsoncxx::types::b_date toBsonDate(Clock::time_point timePoint) {
    return bsoncxx::types::b_date{
        std::chrono::duration_cast<std::chrono::milliseconds>(
            timePoint.time_since_epoch()
        )
    };
}

bsoncxx::document::value toTrackIdentityBindingDocument(
    const domain::TrackIdentityBinding& binding
) {
    bsoncxx::builder::basic::document document;
    document.append(
        bsoncxx::builder::basic::kvp(BindingIdField, binding.bindingId),
        bsoncxx::builder::basic::kvp(TrackIdField, binding.trackId),
        bsoncxx::builder::basic::kvp("employeeId", binding.employeeId),
        bsoncxx::builder::basic::kvp("presenceSessionId", binding.presenceSessionId),
        bsoncxx::builder::basic::kvp("confidence", binding.confidence),
        bsoncxx::builder::basic::kvp("boundAt", toBsonDate(binding.boundAt)),
        bsoncxx::builder::basic::kvp(StatusField, binding.status)
    );

    return document.extract();
}

}

MongoTrackIdentityBindingRepository::MongoTrackIdentityBindingRepository(
    mongocxx::collection trackIdentityBindingsCollection
) : trackIdentityBindingsCollection_{std::move(trackIdentityBindingsCollection)} {
}

std::optional<domain::TrackIdentityBinding> MongoTrackIdentityBindingRepository::findCurrentByTrackId(
    const std::string& trackId
) const {
    bsoncxx::builder::basic::document filter;
    filter.append(
        bsoncxx::builder::basic::kvp(TrackIdField, trackId),
        bsoncxx::builder::basic::kvp(StatusField, ConfirmedStatus)
    );

    auto result = trackIdentityBindingsCollection_.find_one(filter.view());
    if (!result) {
        return std::nullopt;
    }

    return mapTrackIdentityBindingDocument(result->view());
}

void MongoTrackIdentityBindingRepository::create(
    const domain::TrackIdentityBinding& binding
) {
    auto document = toTrackIdentityBindingDocument(binding);
    trackIdentityBindingsCollection_.insert_one(document.view());
}

void MongoTrackIdentityBindingRepository::updateStatus(
    const std::string& bindingId,
    const std::string& status
) {
    bsoncxx::builder::basic::document filter;
    filter.append(bsoncxx::builder::basic::kvp(BindingIdField, bindingId));

    bsoncxx::builder::basic::document updateFields;
    updateFields.append(bsoncxx::builder::basic::kvp(StatusField, status));

    bsoncxx::builder::basic::document update;
    update.append(bsoncxx::builder::basic::kvp("$set", updateFields.extract()));

    trackIdentityBindingsCollection_.update_one(filter.view(), update.view());
}

}
