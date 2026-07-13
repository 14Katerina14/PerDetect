#pragma once

#include <chrono>
#include <optional>
#include <string>
namespace securezone::domain {
enum class AlarmStatus { Created, Active, Acknowledged, Resolved };
struct Alarm {
    std::string alarmId, zoneId, trackId, employeeId, machineId, acknowledgedBy, reason, message;
    AlarmStatus status{AlarmStatus::Created};
    std::chrono::system_clock::time_point enteredAt{}, exitedAt{};
    std::optional<std::chrono::system_clock::time_point> resolvedAt;
    bool stillInside{true};
};
}
