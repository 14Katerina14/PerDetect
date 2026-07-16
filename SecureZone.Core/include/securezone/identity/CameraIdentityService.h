#pragma once

#include "securezone/domain/TrackIdentityBinding.h"
#include "securezone/repository/ICameraObjectTrackRepository.h"
#include "securezone/repository/ITrackIdentityBindingRepository.h"

#include <chrono>
#include <optional>
#include <string>

namespace securezone::identity {

enum class CameraObjectObservationStatus { Active, Lost };

struct CameraObjectObservation {
    std::string cameraId;
    std::string objectId;
    std::string objectType;
    std::chrono::system_clock::time_point observedAt{};
    CameraObjectObservationStatus status{CameraObjectObservationStatus::Active};
};

struct CameraIdentityBindingRequest {
    std::string cameraId;
    std::string employeeId;
    std::string sourceCheckinId;
    std::string presenceSessionId;
    std::chrono::system_clock::time_point boundAt{};
    std::chrono::system_clock::time_point expiresAt{};
};

struct CameraIdentityBindingResult {
    bool bound{};
    std::string status;
    std::string objectId;
    std::string bindingId;
};

class CameraIdentityService {
public:
    CameraIdentityService(
        repository::ICameraObjectTrackRepository& tracks,
        repository::ITrackIdentityBindingRepository& bindings,
        std::chrono::seconds matchingWindow = std::chrono::seconds{15}
    );

    bool observe(const CameraObjectObservation& observation);
    CameraIdentityBindingResult bindLatestHuman(const CameraIdentityBindingRequest& request);
    std::optional<domain::TrackIdentityBinding> resolve(
        const std::string& cameraId,
        const std::string& objectId,
        std::chrono::system_clock::time_point at
    ) const;

private:
    repository::ICameraObjectTrackRepository& tracks_;
    repository::ITrackIdentityBindingRepository& bindings_;
    std::chrono::seconds matchingWindow_;
};

}
