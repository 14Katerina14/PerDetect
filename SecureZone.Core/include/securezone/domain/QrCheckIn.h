#pragma once

#include <chrono>
#include <string>

namespace securezone::domain {

enum class QrCheckInStatus { Active, Expired, Revoked };

// A QR scan is both the audit record and the time-bounded identity evidence
// used for track association.  The alias below keeps the repository naming
// compatible without creating a second, divergent model.
struct QrCheckIn {
    std::string checkInId;
    std::string employeeId;
    std::string zoneId;
    std::string deviceId;
    QrCheckInStatus status{QrCheckInStatus::Active};
    std::chrono::system_clock::time_point scannedAt{};
    std::chrono::system_clock::time_point validUntil{};

    bool hasValidWindow() const {
        return !checkInId.empty()
            && !employeeId.empty()
            && !zoneId.empty()
            && validUntil > scannedAt;
    }

    bool isActiveAt(std::chrono::system_clock::time_point at) const {
        return status == QrCheckInStatus::Active && hasValidWindow()
            && scannedAt <= at && at <= validUntil;
    }
};

using QrCheckin = QrCheckIn;

}
