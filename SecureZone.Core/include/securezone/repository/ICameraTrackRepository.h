#pragma once

#include <chrono>
#include <optional>
#include <string>

#include "securezone/domain/CameraTrack.h"

namespace securezone::repository {

class ICameraTrackRepository {
public:
    virtual ~ICameraTrackRepository() = default;

    virtual std::optional<domain::CameraTrack> findByTrackId(
        const std::string& trackId
    ) const = 0;

    virtual void upsert(const domain::CameraTrack& cameraTrack) = 0;

    virtual void markLost(
        const std::string& trackId,
        std::chrono::system_clock::time_point lostAt
    ) = 0;
};

}
