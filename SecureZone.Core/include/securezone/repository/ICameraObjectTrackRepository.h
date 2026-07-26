#pragma once

#include "securezone/domain/CameraObjectTrack.h"

#include <chrono>
#include <optional>
#include <string>
#include <vector>

namespace securezone::repository {

class ICameraObjectTrackRepository {
public:
    virtual ~ICameraObjectTrackRepository() = default;

    virtual void upsertObservation(const domain::CameraObjectTrack& track) = 0;
    virtual std::optional<domain::CameraObjectTrack> findByCameraAndObject(
        const std::string& cameraId,
        const std::string& objectId
    ) const {
        (void)cameraId;
        (void)objectId;
        return std::nullopt;
    }
    virtual void markLost(
        const std::string& cameraId,
        const std::string& objectId,
        std::chrono::system_clock::time_point lostAt
    ) {
        (void)cameraId;
        (void)objectId;
        (void)lostAt;
    }
    virtual std::vector<domain::CameraObjectTrack> findRecentHumans(
        const std::string& cameraId,
        std::chrono::system_clock::time_point seenAfter,
        std::chrono::system_clock::time_point seenAtOrBefore
    ) const = 0;
};

}
