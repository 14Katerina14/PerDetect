#pragma once

#include "securezone/domain/PresenceSession.h"

#include <chrono>
#include <cstddef>
#include <optional>
#include <string>
#include <vector>

namespace securezone::repository {

class IPresenceSessionReadRepository {
public:
    virtual ~IPresenceSessionReadRepository() = default;

    virtual std::vector<domain::PresenceSession> findActiveAt(
        std::chrono::system_clock::time_point at,
        std::size_t limit,
        const std::optional<std::string>& zoneId = std::nullopt
    ) const = 0;
};

}
