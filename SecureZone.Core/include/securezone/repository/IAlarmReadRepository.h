#pragma once

#include "securezone/domain/Alarm.h"

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

namespace securezone::repository {

class IAlarmReadRepository {
public:
    virtual ~IAlarmReadRepository() = default;

    virtual std::vector<domain::Alarm> findActive(
        std::size_t limit,
        const std::optional<std::string>& zoneId = std::nullopt
    ) const = 0;
};

}
