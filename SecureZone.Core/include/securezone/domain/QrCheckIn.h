#pragma once

#include <chrono>
#include <string>
namespace securezone::domain {
enum class QrCheckInStatus { Active, Expired, Revoked };
struct QrCheckIn { std::string checkInId, employeeId, zoneId, deviceId; QrCheckInStatus status{QrCheckInStatus::Active}; std::chrono::system_clock::time_point scannedAt{}, validUntil{}; };
}
