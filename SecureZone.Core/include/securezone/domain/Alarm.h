#pragma once

#include <chrono>
#include <string>
namespace securezone::domain {
enum class AlarmStatus { Created, Active, Acknowledged, Resolved };
enum class AlarmSeverity { Low, Medium, High, Critical };
struct Alarm { std::string alarmId, zoneId, trackId, employeeId, machineId, acknowledgedBy, message; AlarmStatus status{AlarmStatus::Created}; AlarmSeverity severity{AlarmSeverity::High}; std::chrono::system_clock::time_point enteredAt{}, exitedAt{}; bool stillInside{true}; };
}
