#pragma once

#include <chrono>
#include <cstddef>
#include <optional>
#include <string>

#include "securezone/domain/Alarm.h"

namespace securezone::repository {

class IAlarmRepository {
public:
    virtual ~IAlarmRepository() = default;

    virtual std::optional<domain::Alarm> findActiveByTrackAndZone(
        const std::string& trackId,
        const std::string& zoneId
    ) const = 0;

    virtual std::size_t countActiveByZone(const std::string& zoneId) const = 0;

    virtual void create(const domain::Alarm& alarm) = 0;

    virtual void resolve(
        const std::string& alarmId,
        std::chrono::system_clock::time_point resolvedAt
    ) = 0;
};

}
