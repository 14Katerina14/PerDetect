#pragma once

#include "securezone/domain/CameraObjectTrack.h"

#include <chrono>
#include <string>
#include <vector>

namespace securezone::repository {

class ICameraObjectTrackRepository {
public:
    virtual ~ICameraObjectTrackRepository() = default;

    virtual void upsertObservation(const domain::CameraObjectTrack& track) = 0;
    virtual std::vector<domain::CameraObjectTrack> findRecentHumans(
        const std::string& cameraId,
        std::chrono::system_clock::time_point seenAfter,
        std::chrono::system_clock::time_point seenAtOrBefore
    ) const = 0;
};

}
