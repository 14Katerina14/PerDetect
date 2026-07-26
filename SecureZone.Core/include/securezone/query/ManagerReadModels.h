#pragma once

#include "securezone/domain/Alarm.h"
#include "securezone/domain/MachineState.h"
#include "securezone/domain/PresenceSession.h"
#include "securezone/domain/Zone.h"

#include <chrono>
#include <optional>
#include <string>

namespace securezone::query {

struct AlarmView {
    std::string alarmId;
    std::string zoneId;
    std::string zoneName;
    std::string trackId;
    std::string employeeId;
    std::string employeeName;
    std::string machineId;
    std::string machineName;
    std::optional<domain::MachineStatus> machineStatus;
    domain::AlarmStatus status{domain::AlarmStatus::Active};
    std::string reason;
    std::string message;
    std::chrono::system_clock::time_point enteredAt{};
    std::chrono::system_clock::time_point exitedAt{};
    std::optional<std::chrono::system_clock::time_point> resolvedAt;
    bool stillInside{};
};

using ActiveAlarmView = AlarmView;

struct ActivePresenceView {
    std::string sessionId;
    std::string employeeId;
    std::string employeeName;
    std::string zoneId;
    std::string zoneName;
    std::chrono::system_clock::time_point startedAt{};
    std::chrono::system_clock::time_point expiresAt{};
};

struct ZoneStatusView {
    std::string zoneId;
    std::string zoneName;
    std::string cameraId;
    domain::ZoneType type{domain::ZoneType::Restricted};
    domain::ZoneStatus status{domain::ZoneStatus::Active};
    std::string machineId;
    std::string machineName;
    std::optional<domain::MachineStatus> machineStatus;
};

}
