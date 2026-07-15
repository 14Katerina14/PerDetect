#include "securezone/identity/CameraIdentityService.h"

#include <algorithm>
#include <cassert>
#include <vector>

using namespace securezone;

namespace {

struct TrackRepository final : repository::ICameraObjectTrackRepository {
    std::vector<domain::CameraObjectTrack> tracks;

    void upsertObservation(const domain::CameraObjectTrack& observation) override {
        auto existing = std::find_if(tracks.begin(), tracks.end(), [&](const auto& track) {
            return track.cameraId == observation.cameraId && track.objectId == observation.objectId;
        });
        if (existing == tracks.end()) {
            tracks.push_back(observation);
        } else {
            existing->lastSeenAt = observation.lastSeenAt;
        }
    }

    std::vector<domain::CameraObjectTrack> findRecentHumans(
        const std::string& cameraId,
        std::chrono::system_clock::time_point after,
        std::chrono::system_clock::time_point before
    ) const override {
        std::vector<domain::CameraObjectTrack> result;
        for (const auto& track : tracks) {
            if (track.cameraId == cameraId && track.isHuman()
                && track.firstSeenAt >= after && track.firstSeenAt <= before) result.push_back(track);
        }
        std::sort(result.begin(), result.end(), [](const auto& left, const auto& right) {
            return left.firstSeenAt > right.firstSeenAt;
        });
        return result;
    }
};

struct BindingRepository final : repository::ITrackIdentityBindingRepository {
    std::vector<domain::TrackIdentityBinding> bindings;

    std::optional<domain::TrackIdentityBinding> findActiveByTrack(
        const std::string& cameraId, const std::string& objectId,
        std::chrono::system_clock::time_point at
    ) const override {
        for (const auto& binding : bindings) {
            if (binding.cameraId == cameraId && binding.objectId == objectId && binding.isActiveAt(at)) return binding;
        }
        return std::nullopt;
    }

    void create(const domain::TrackIdentityBinding& binding) override { bindings.push_back(binding); }
};

}

int main() {
    using Clock = std::chrono::system_clock;
    const auto now = Clock::time_point{std::chrono::seconds{100}};
    TrackRepository tracks;
    BindingRepository bindings;
    identity::CameraIdentityService service{tracks, bindings, std::chrono::seconds{15}};

    assert(service.observe({"CAM-1", "41", "Human", now - std::chrono::seconds{5}}));
    assert(service.observe({"CAM-1", "42", "Human", now - std::chrono::seconds{2}}));
    assert(!service.observe({"", "43", "Human", now}));

    auto result = service.bindLatestHuman({
        "CAM-1", "EMP-1", "CHECK-1", "SESSION-1", now, now + std::chrono::minutes{2}
    });
    assert(result.bound);
    assert(result.objectId == "42");
    assert(service.resolve("CAM-1", "42", now).value().employeeId == "EMP-1");

    auto second = service.bindLatestHuman({
        "CAM-1", "EMP-2", "CHECK-2", "SESSION-2", now, now + std::chrono::minutes{2}
    });
    assert(second.bound && second.objectId == "41");

    auto none = service.bindLatestHuman({
        "CAM-2", "EMP-3", "CHECK-3", "SESSION-3", now, now + std::chrono::minutes{2}
    });
    assert(!none.bound && none.status == "no_recent_human");
}
