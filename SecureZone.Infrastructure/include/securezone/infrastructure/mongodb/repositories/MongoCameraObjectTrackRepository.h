#pragma once

#include "securezone/repository/ICameraObjectTrackRepository.h"
#include <mongocxx/collection.hpp>

namespace securezone::infrastructure::mongodb::repositories {

class MongoCameraObjectTrackRepository final : public repository::ICameraObjectTrackRepository {
public:
    explicit MongoCameraObjectTrackRepository(mongocxx::collection collection);
    void upsertObservation(const domain::CameraObjectTrack& track) override;
    std::optional<domain::CameraObjectTrack> findByCameraAndObject(
        const std::string& cameraId,
        const std::string& objectId
    ) const override;
    void markLost(
        const std::string& cameraId,
        const std::string& objectId,
        std::chrono::system_clock::time_point lostAt
    ) override;
    std::vector<domain::CameraObjectTrack> findRecentHumans(
        const std::string& cameraId,
        std::chrono::system_clock::time_point seenAfter,
        std::chrono::system_clock::time_point seenAtOrBefore
    ) const override;

private:
    mutable mongocxx::collection collection_;
};

}
