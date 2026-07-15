#pragma once

#include "securezone/domain/TrackIdentityBinding.h"

#include <chrono>
#include <optional>
#include <string>

namespace securezone::repository {

class ITrackIdentityBindingRepository {
public:
    virtual ~ITrackIdentityBindingRepository() = default;

    virtual std::optional<domain::TrackIdentityBinding> findActiveByTrack(
        const std::string& cameraId,
        const std::string& objectId,
        std::chrono::system_clock::time_point at
    ) const = 0;

    virtual void create(const domain::TrackIdentityBinding& binding) = 0;
};

}
