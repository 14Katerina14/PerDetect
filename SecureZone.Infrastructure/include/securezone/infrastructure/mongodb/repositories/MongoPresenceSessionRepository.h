#pragma once

#include <mongocxx/collection.hpp>

#include <chrono>
#include <optional>
#include <string>

#include "securezone/domain/PresenceSession.h"
#include "securezone/repository/IPresenceSessionRepository.h"
#include "securezone/repository/IPresenceSessionReadRepository.h"

namespace securezone::infrastructure::mongodb::repositories {

class MongoPresenceSessionRepository final
    : public repository::IPresenceSessionRepository,
      public repository::IPresenceSessionReadRepository {
public:
    explicit MongoPresenceSessionRepository(mongocxx::collection presenceSessionsCollection);

    std::optional<domain::PresenceSession> findActiveByEmployeeAndZone(
        const std::string& employeeId,
        const std::string& zoneId
    ) const override;

    std::optional<domain::PresenceSession> findActiveByZoneAt(
        const std::string& zoneId,
        std::chrono::system_clock::time_point at
    ) const override;

    std::vector<domain::PresenceSession> findActiveAt(
        std::chrono::system_clock::time_point at,
        std::size_t limit,
        const std::optional<std::string>& zoneId = std::nullopt
    ) const override;

    void create(const domain::PresenceSession& presenceSession) override;

    void extend(
        const std::string& sessionId,
        std::chrono::system_clock::time_point expiresAt
    ) override;

    void end(
        const std::string& sessionId,
        std::chrono::system_clock::time_point endedAt
    ) override;

private:
    mutable mongocxx::collection presenceSessionsCollection_;
};

}
