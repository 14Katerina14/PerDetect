#include "securezone/identity/UnidentifiedPersonWatchdog.h"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <optional>
#include <string>
#include <vector>

namespace {

using namespace securezone;
using Clock = std::chrono::system_clock;
const auto FirstSeen = Clock::time_point{} + std::chrono::hours{1};

class TrackRepository final : public repository::ICameraObjectTrackRepository {
public:
    void upsertObservation(const domain::CameraObjectTrack& incoming) override {
        auto track = findMutable(incoming.cameraId, incoming.objectId);
        if (track == tracks.end()) {
            tracks.push_back(incoming);
            return;
        }
        track->lastSeenAt = incoming.lastSeenAt;
        track->status = domain::CameraObjectTrackStatus::Active;
    }

    std::optional<domain::CameraObjectTrack> findByCameraAndObject(
        const std::string& cameraId,
        const std::string& objectId
    ) const override {
        const auto track = find(cameraId, objectId);
        return track == tracks.end() ? std::nullopt
                                     : std::optional<domain::CameraObjectTrack>{*track};
    }

    void markLost(
        const std::string& cameraId,
        const std::string& objectId,
        Clock::time_point lostAt
    ) override {
        auto track = findMutable(cameraId, objectId);
        if (track == tracks.end()) return;
        track->lastSeenAt = lostAt;
        track->status = domain::CameraObjectTrackStatus::Lost;
    }

    std::vector<domain::CameraObjectTrack> findRecentHumans(
        const std::string&,
        Clock::time_point,
        Clock::time_point
    ) const override {
        return {};
    }

    std::vector<domain::CameraObjectTrack> tracks;

private:
    std::vector<domain::CameraObjectTrack>::iterator findMutable(
        const std::string& cameraId,
        const std::string& objectId
    ) {
        return std::find_if(tracks.begin(), tracks.end(), [&](const auto& track) {
            return track.cameraId == cameraId && track.objectId == objectId;
        });
    }

    std::vector<domain::CameraObjectTrack>::const_iterator find(
        const std::string& cameraId,
        const std::string& objectId
    ) const {
        return std::find_if(tracks.begin(), tracks.end(), [&](const auto& track) {
            return track.cameraId == cameraId && track.objectId == objectId;
        });
    }
};

class BindingRepository final : public repository::ITrackIdentityBindingRepository {
public:
    std::optional<domain::TrackIdentityBinding> findActiveByTrack(
        const std::string& cameraId,
        const std::string& objectId,
        Clock::time_point at
    ) const override {
        const auto binding = std::find_if(bindings.begin(), bindings.end(), [&](const auto& current) {
            return current.cameraId == cameraId && current.objectId == objectId
                && current.isActiveAt(at);
        });
        return binding == bindings.end() ? std::nullopt
                                         : std::optional<domain::TrackIdentityBinding>{*binding};
    }

    void create(const domain::TrackIdentityBinding& binding) override {
        bindings.push_back(binding);
    }

    std::vector<domain::TrackIdentityBinding> bindings;
};

class ZoneRepository final : public repository::IZoneRepository {
public:
    ZoneRepository() {
        zone.zoneId = "SAFE-001";
        zone.name = "Factory entrance";
        zone.cameraId = "CAM-001";
        zone.type = domain::ZoneType::Safe;
        zone.status = domain::ZoneStatus::Active;
    }

    std::optional<domain::Zone> findByZoneId(const std::string&) const override { return zone; }
    std::optional<domain::Zone> findActiveByZoneId(const std::string&) const override { return zone; }
    std::optional<domain::Zone> findActiveSafeByCameraId(
        const std::string& cameraId
    ) const override {
        return cameraId == zone.cameraId ? std::optional<domain::Zone>{zone} : std::nullopt;
    }
    bool save(const domain::Zone& value) override { zone = value; return true; }

    domain::Zone zone;
};

class AlarmRepository final : public repository::IAlarmRepository {
public:
    std::optional<domain::Alarm> findActiveByTrackAndZone(
        const std::string& trackId,
        const std::string& zoneId
    ) const override {
        const auto alarm = std::find_if(alarms.begin(), alarms.end(), [&](const auto& current) {
            return current.trackId == trackId && current.zoneId == zoneId
                && current.status != domain::AlarmStatus::Resolved;
        });
        return alarm == alarms.end() ? std::nullopt : std::optional<domain::Alarm>{*alarm};
    }

    std::size_t countActiveByZone(const std::string& zoneId) const override {
        return static_cast<std::size_t>(std::count_if(
            alarms.begin(), alarms.end(), [&](const auto& alarm) {
                return alarm.zoneId == zoneId && alarm.status != domain::AlarmStatus::Resolved;
            }
        ));
    }

    void create(const domain::Alarm& alarm) override { alarms.push_back(alarm); }

    void resolve(const std::string& alarmId, Clock::time_point resolvedAt) override {
        auto alarm = std::find_if(alarms.begin(), alarms.end(), [&](const auto& current) {
            return current.alarmId == alarmId;
        });
        assert(alarm != alarms.end());
        alarm->status = domain::AlarmStatus::Resolved;
        alarm->resolvedAt = resolvedAt;
        alarm->stillInside = false;
    }

    std::vector<domain::Alarm> alarms;
};

identity::CameraObjectObservation observation(
    Clock::time_point at,
    identity::CameraObjectObservationStatus status = identity::CameraObjectObservationStatus::Active
) {
    return {"CAM-001", "OBJECT-42", "Human", at, status};
}

struct Fixture {
    TrackRepository tracks;
    BindingRepository bindings;
    ZoneRepository zones;
    AlarmRepository alarms;
    identity::CameraIdentityService identities{tracks, bindings};
    int createdNotifications{};
    int resolvedNotifications{};
    identity::UnidentifiedPersonWatchdog watchdog{
        identities,
        tracks,
        zones,
        alarms,
        [this](const domain::Alarm&) { ++createdNotifications; },
        [this](const domain::Alarm& alarm) {
            assert(alarm.status == domain::AlarmStatus::Resolved);
            assert(!alarm.stillInside);
            assert(alarm.resolvedAt.has_value());
            ++resolvedNotifications;
        },
        std::chrono::minutes{2}
    };
};

void waitsForTwoMinuteIdentityGracePeriod() {
    Fixture fixture;
    const auto first = fixture.watchdog.observe(observation(FirstSeen));
    const auto pending = fixture.watchdog.observe(observation(FirstSeen + std::chrono::seconds{119}));

    assert(first.decision == "pending_identity");
    assert(pending.decision == "pending_identity");
    assert(fixture.alarms.alarms.empty());
    assert(fixture.createdNotifications == 0);
    assert(fixture.resolvedNotifications == 0);
}

void createsOneAlarmAndQueuesOneSupervisorNotification() {
    Fixture fixture;
    fixture.watchdog.observe(observation(FirstSeen));
    const auto violation = fixture.watchdog.observe(observation(FirstSeen + std::chrono::minutes{2}));
    const auto duplicate = fixture.watchdog.observe(observation(FirstSeen + std::chrono::seconds{125}));

    assert(violation.decision == "violation");
    assert(!violation.duplicate);
    assert(duplicate.decision == "violation_active");
    assert(duplicate.duplicate);
    assert(fixture.alarms.alarms.size() == 1);
    assert(fixture.createdNotifications == 1);
    assert(fixture.resolvedNotifications == 0);
    assert(fixture.alarms.alarms.front().employeeId.empty());
}

void activeQrBindingPreventsUnknownIdentityAlarm() {
    Fixture fixture;
    fixture.watchdog.observe(observation(FirstSeen));
    domain::TrackIdentityBinding binding{};
    binding.cameraId = "CAM-001";
    binding.objectId = "OBJECT-42";
    binding.employeeId = "EMP-001";
    binding.boundAt = FirstSeen + std::chrono::seconds{10};
    binding.expiresAt = FirstSeen + std::chrono::minutes{5};
    binding.status = domain::TrackIdentityBindingStatus::Active;
    fixture.bindings.bindings.push_back(binding);

    const auto identified = fixture.watchdog.observe(
        observation(FirstSeen + std::chrono::minutes{2})
    );
    assert(identified.decision == "identified");
    assert(fixture.alarms.alarms.empty());
}

void leavingClearsTheLastUnknownIdentityAlarm() {
    Fixture fixture;
    fixture.watchdog.observe(observation(FirstSeen));
    fixture.watchdog.observe(observation(FirstSeen + std::chrono::minutes{2}));

    const auto cleared = fixture.watchdog.observe(observation(
        FirstSeen + std::chrono::seconds{130},
        identity::CameraObjectObservationStatus::Lost
    ));

    assert(cleared.decision == "cleared");
    assert(fixture.alarms.alarms.front().status == domain::AlarmStatus::Resolved);
    assert(!fixture.alarms.alarms.front().stillInside);
    assert(fixture.resolvedNotifications == 1);
}

void lateQrIdentificationClearsExistingAlarm() {
    Fixture fixture;
    fixture.watchdog.observe(observation(FirstSeen));
    fixture.watchdog.observe(observation(FirstSeen + std::chrono::minutes{2}));
    domain::TrackIdentityBinding binding{};
    binding.cameraId = "CAM-001";
    binding.objectId = "OBJECT-42";
    binding.employeeId = "EMP-001";
    binding.boundAt = FirstSeen + std::chrono::seconds{125};
    binding.expiresAt = FirstSeen + std::chrono::minutes{5};
    binding.status = domain::TrackIdentityBindingStatus::Active;
    fixture.bindings.bindings.push_back(binding);

    const auto cleared = fixture.watchdog.observe(observation(FirstSeen + std::chrono::seconds{130}));
    assert(cleared.decision == "cleared");
    assert(fixture.alarms.alarms.front().status == domain::AlarmStatus::Resolved);
    assert(fixture.resolvedNotifications == 1);
}

}

int main() {
    waitsForTwoMinuteIdentityGracePeriod();
    createsOneAlarmAndQueuesOneSupervisorNotification();
    activeQrBindingPreventsUnknownIdentityAlarm();
    leavingClearsTheLastUnknownIdentityAlarm();
    lateQrIdentificationClearsExistingAlarm();
}
