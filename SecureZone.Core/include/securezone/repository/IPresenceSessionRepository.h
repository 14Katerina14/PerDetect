#pragma once

#include <chrono>
#include <optional>
#include <string>

#include "securezone/domain/PresenceSession.h"

namespace securezone::repository {

class IPresenceSessionRepository {
public:
    virtual ~IPresenceSessionRepository() = default;

    virtual std::optional<domain::PresenceSession> findActiveByEmployeeAndZone(
        const std::string& employeeId,
        const std::string& zoneId
    ) const = 0;

    virtual std::optional<domain::PresenceSession> findActiveByZoneAt(
        const std::string& zoneId,
        std::chrono::system_clock::time_point at
    ) const {
        (void)zoneId;
        (void)at;
        return std::nullopt;
    }

    virtual void create(const domain::PresenceSession& presenceSession) = 0;

    virtual void extend(
        const std::string& sessionId,
        std::chrono::system_clock::time_point expiresAt
    ) = 0;

    virtual void end(
        const std::string& sessionId,
        std::chrono::system_clock::time_point endedAt
    ) = 0;
};

}
