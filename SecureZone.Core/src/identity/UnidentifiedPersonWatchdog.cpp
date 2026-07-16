#include "securezone/identity/UnidentifiedPersonWatchdog.h"

#include <chrono>
#include <utility>

namespace securezone::identity {
namespace {

using Clock = std::chrono::system_clock;

std::string trackId(const CameraObjectObservation& observation) {
    return observation.cameraId + ":" + observation.objectId;
}

std::string eventId(const CameraObjectObservation& observation) {
    return "IDENTITY-" + observation.cameraId + "-" + observation.objectId;
}

UnidentifiedPersonWatchdogResult result(
    const CameraObjectObservation& observation,
    bool accepted,
    std::string status,
    std::string decision,
    std::string zoneId,
    std::string message,
    bool duplicate = false
) {
    return {
        accepted,
        std::move(status),
        std::move(decision),
        std::move(zoneId),
        std::move(message),
        eventId(observation),
        duplicate
    };
}

domain::Alarm createAlarm(
    const CameraObjectObservation& observation,
    const domain::Zone& zone
) {
    domain::Alarm alarm{};
    alarm.alarmId = "ALARM-" + trackId(observation) + "-" + zone.zoneId;
    alarm.zoneId = zone.zoneId;
    alarm.trackId = trackId(observation);
    alarm.status = domain::AlarmStatus::Active;
    alarm.reason = "A person remained unidentified after the QR grace period.";
    alarm.message = "Unidentified person " + observation.objectId +
        " is still visible on camera " + observation.cameraId +
        " in zone " + zone.name + ".";
    alarm.enteredAt = observation.observedAt;
    alarm.stillInside = true;
    return alarm;
}

}

UnidentifiedPersonWatchdog::UnidentifiedPersonWatchdog(
    CameraIdentityService& identityService,
    repository::ICameraObjectTrackRepository& trackRepository,
    repository::IZoneRepository& zoneRepository,
    repository::IAlarmRepository& alarmRepository,
    AlarmCreatedNotifier alarmCreatedNotifier,
    AlarmResolvedNotifier alarmResolvedNotifier,
    std::chrono::seconds identityGracePeriod
) : identityService_{identityService},
    trackRepository_{trackRepository},
    zoneRepository_{zoneRepository},
    alarmRepository_{alarmRepository},
    alarmCreatedNotifier_{std::move(alarmCreatedNotifier)},
    alarmResolvedNotifier_{std::move(alarmResolvedNotifier)},
    identityGracePeriod_{identityGracePeriod} {
}

void UnidentifiedPersonWatchdog::resolveAlarm(
    const domain::Alarm& alarm,
    Clock::time_point resolvedAt
) {
    alarmRepository_.resolve(alarm.alarmId, resolvedAt);
    if (!alarmResolvedNotifier_) return;

    auto resolvedAlarm = alarm;
    resolvedAlarm.status = domain::AlarmStatus::Resolved;
    resolvedAlarm.stillInside = false;
    resolvedAlarm.exitedAt = resolvedAt;
    resolvedAlarm.resolvedAt = resolvedAt;
    alarmResolvedNotifier_(resolvedAlarm);
}

UnidentifiedPersonWatchdogResult UnidentifiedPersonWatchdog::observe(
    const CameraObjectObservation& observation
) {
    if (observation.cameraId.empty() || observation.objectId.empty()
        || observation.objectType.empty() || observation.observedAt == Clock::time_point{}) {
        return result(observation, false, "invalid_request", "none", {},
            "Camera observation is incomplete.");
    }

    if (!identityService_.observe(observation)) {
        return result(observation, false, "rejected", "none", {},
            "Camera observation was rejected.");
    }

    const auto zone = zoneRepository_.findActiveSafeByCameraId(observation.cameraId);
    if (!zone.has_value()) {
        return result(observation, true, "observed", "none", {},
            "No active safe zone is configured for this camera.");
    }

    const auto currentTrackId = trackId(observation);
    const auto activeAlarm = alarmRepository_.findActiveByTrackAndZone(
        currentTrackId,
        zone->zoneId
    );

    if (observation.status == CameraObjectObservationStatus::Lost) {
        if (!activeAlarm.has_value()) {
            return result(observation, true, "processed", "none", zone->zoneId,
                "The camera object left without an active unidentified-person alarm.");
        }

        resolveAlarm(*activeAlarm, observation.observedAt);
        if (alarmRepository_.countActiveByZone(zone->zoneId) == 0U) {
            return result(observation, true, "processed", "cleared", zone->zoneId,
                "The unidentified person left and no active violations remain in the zone.");
        }

        return result(observation, true, "processed", "violation_active", zone->zoneId,
            "The person left, but other active violations remain in the zone.");
    }

    const auto track = trackRepository_.findByCameraAndObject(
        observation.cameraId,
        observation.objectId
    );
    if (!track.has_value() || !track->isHuman()) {
        return result(observation, true, "observed", "none", zone->zoneId,
            "The observation is not a tracked Human object.");
    }

    const auto binding = identityService_.resolve(
        observation.cameraId,
        observation.objectId,
        observation.observedAt
    );
    if (binding.has_value()) {
        if (activeAlarm.has_value()) {
            resolveAlarm(*activeAlarm, observation.observedAt);
            if (alarmRepository_.countActiveByZone(zone->zoneId) == 0U) {
                return result(observation, true, "processed", "cleared", zone->zoneId,
                    "The camera object was identified and no active violations remain.");
            }
        }

        return result(observation, true, "processed", "identified", zone->zoneId,
            "The camera object has an active QR identity binding.");
    }

    if (observation.observedAt < track->firstSeenAt + identityGracePeriod_) {
        return result(observation, true, "processed", "pending_identity", zone->zoneId,
            "The QR identity grace period is still active.");
    }

    if (activeAlarm.has_value()) {
        return result(observation, true, "processed", "violation_active", zone->zoneId,
            activeAlarm->reason, true);
    }

    auto alarm = createAlarm(observation, *zone);
    alarm.enteredAt = track->firstSeenAt + identityGracePeriod_;
    alarmRepository_.create(alarm);
    if (alarmCreatedNotifier_) {
        alarmCreatedNotifier_(alarm);
    }

    return result(observation, true, "processed", "violation", zone->zoneId,
        alarm.reason);
}

}
