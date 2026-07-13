#pragma once

#include <chrono>
#include <string>

namespace securezone::domain {

enum class PresenceSessionStatus { Active, Ended, Expired };

// A time-bounded authorization presence created from a successful QR scan.
struct PresenceSession {
    std::string sessionId;
    std::string employeeId;
    std::string zoneId;
    std::string sourceCheckinId;
    std::chrono::system_clock::time_point startedAt{};
    std::chrono::system_clock::time_point expiresAt{};
    std::chrono::system_clock::time_point endedAt{};
    PresenceSessionStatus status{PresenceSessionStatus::Active};

    bool hasValidWindow() const {
        return !sessionId.empty()
            && !employeeId.empty()
            && !zoneId.empty()
            && !sourceCheckinId.empty()
            && expiresAt > startedAt;
    }

    bool isActiveAt(std::chrono::system_clock::time_point at) const {
        return status == PresenceSessionStatus::Active && hasValidWindow()
            && startedAt <= at && at <= expiresAt;
    }

    bool canExtendTo(std::chrono::system_clock::time_point newExpiresAt) const {
        return status == PresenceSessionStatus::Active && newExpiresAt >= expiresAt;
    }
};

}
