#pragma once

#include "securezone/domain/Alarm.h"
#include "securezone/identity/CameraIdentityService.h"
#include "securezone/repository/IAlarmRepository.h"
#include "securezone/repository/ICameraObjectTrackRepository.h"
#include "securezone/repository/IZoneRepository.h"

#include <chrono>
#include <functional>
#include <string>

namespace securezone::identity {

struct UnidentifiedPersonWatchdogResult {
    bool accepted{};
    std::string status;
    std::string decision;
    std::string zoneId;
    std::string message;
    std::string eventId;
    bool duplicate{};
};

class UnidentifiedPersonWatchdog {
public:
    using AlarmCreatedNotifier = std::function<void(const domain::Alarm&)>;
    using AlarmResolvedNotifier = std::function<void(const domain::Alarm&)>;

    UnidentifiedPersonWatchdog(
        CameraIdentityService& identityService,
        repository::ICameraObjectTrackRepository& trackRepository,
        repository::IZoneRepository& zoneRepository,
        repository::IAlarmRepository& alarmRepository,
        AlarmCreatedNotifier alarmCreatedNotifier = {},
        AlarmResolvedNotifier alarmResolvedNotifier = {},
        std::chrono::seconds identityGracePeriod = std::chrono::minutes{2}
    );

    UnidentifiedPersonWatchdogResult observe(const CameraObjectObservation& observation);

private:
    CameraIdentityService& identityService_;
    repository::ICameraObjectTrackRepository& trackRepository_;
    repository::IZoneRepository& zoneRepository_;
    repository::IAlarmRepository& alarmRepository_;
    AlarmCreatedNotifier alarmCreatedNotifier_;
    AlarmResolvedNotifier alarmResolvedNotifier_;
    std::chrono::seconds identityGracePeriod_;

    void resolveAlarm(
        const domain::Alarm& alarm,
        std::chrono::system_clock::time_point resolvedAt
    );
};

}
