#pragma once

#include "securezone/repository/ITrackIdentityBindingRepository.h"
#include <mongocxx/collection.hpp>

namespace securezone::infrastructure::mongodb::repositories {

class MongoTrackIdentityBindingRepository final : public repository::ITrackIdentityBindingRepository {
public:
    explicit MongoTrackIdentityBindingRepository(mongocxx::collection collection);
    std::optional<domain::TrackIdentityBinding> findActiveByTrack(
        const std::string& cameraId,
        const std::string& objectId,
        std::chrono::system_clock::time_point at
    ) const override;
    void create(const domain::TrackIdentityBinding& binding) override;

private:
    mutable mongocxx::collection collection_;
};

}
