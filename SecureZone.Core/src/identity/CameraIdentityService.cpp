#include "securezone/identity/CameraIdentityService.h"

#include <chrono>

namespace securezone::identity {
namespace {

std::string bindingId(const CameraIdentityBindingRequest& request, const std::string& objectId) {
    const auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(
        request.boundAt.time_since_epoch()
    ).count();
    return "BINDING-" + request.cameraId + "-" + objectId + "-" + std::to_string(millis);
}

}

CameraIdentityService::CameraIdentityService(
    repository::ICameraObjectTrackRepository& tracks,
    repository::ITrackIdentityBindingRepository& bindings,
    std::chrono::seconds matchingWindow
) : tracks_{tracks}, bindings_{bindings}, matchingWindow_{matchingWindow} {
}

bool CameraIdentityService::observe(const CameraObjectObservation& observation) {
    if (observation.cameraId.empty() || observation.objectId.empty()
        || observation.objectType.empty() || observation.observedAt == std::chrono::system_clock::time_point{}) {
        return false;
    }

    if (observation.status == CameraObjectObservationStatus::Lost) {
        tracks_.markLost(observation.cameraId, observation.objectId, observation.observedAt);
        return true;
    }

    domain::CameraObjectTrack track{};
    track.cameraId = observation.cameraId;
    track.objectId = observation.objectId;
    track.objectType = observation.objectType;
    track.firstSeenAt = observation.observedAt;
    track.lastSeenAt = observation.observedAt;
    tracks_.upsertObservation(track);
    return true;
}

CameraIdentityBindingResult CameraIdentityService::bindLatestHuman(
    const CameraIdentityBindingRequest& request
) {
    if (request.cameraId.empty() || request.employeeId.empty() || request.sourceCheckinId.empty()
        || request.presenceSessionId.empty() || request.expiresAt <= request.boundAt) {
        return {false, "invalid_request", {}, {}};
    }

    const auto candidates = tracks_.findRecentHumans(
        request.cameraId,
        request.boundAt - matchingWindow_,
        request.boundAt
    );
    for (const auto& candidate : candidates) {
        if (!candidate.isHuman() || bindings_.findActiveByTrack(
                candidate.cameraId, candidate.objectId, request.boundAt).has_value()) {
            continue;
        }

        domain::TrackIdentityBinding binding{};
        binding.bindingId = bindingId(request, candidate.objectId);
        binding.cameraId = candidate.cameraId;
        binding.objectId = candidate.objectId;
        binding.employeeId = request.employeeId;
        binding.sourceCheckinId = request.sourceCheckinId;
        binding.presenceSessionId = request.presenceSessionId;
        binding.boundAt = request.boundAt;
        binding.expiresAt = request.expiresAt;
        bindings_.create(binding);
        return {true, "bound", binding.objectId, binding.bindingId};
    }

    return {false, "no_recent_human", {}, {}};
}

std::optional<domain::TrackIdentityBinding> CameraIdentityService::resolve(
    const std::string& cameraId,
    const std::string& objectId,
    std::chrono::system_clock::time_point at
) const {
    if (cameraId.empty() || objectId.empty()) {
        return std::nullopt;
    }
    return bindings_.findActiveByTrack(cameraId, objectId, at);
}

}
