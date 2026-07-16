#pragma once

#include <mongocxx/collection.hpp>

#include <chrono>
#include <optional>
#include <string>

#include "securezone/domain/Alarm.h"
#include "securezone/repository/IAlarmRepository.h"
#include "securezone/repository/IAlarmReadRepository.h"

namespace securezone::infrastructure::mongodb::repositories {

class MongoAlarmRepository final
    : public repository::IAlarmRepository,
      public repository::IAlarmReadRepository {
public:
    explicit MongoAlarmRepository(mongocxx::collection alarmsCollection);

    std::optional<domain::Alarm> findActiveByTrackAndZone(
        const std::string& trackId,
        const std::string& zoneId
    ) const override;

    std::size_t countActiveByZone(const std::string& zoneId) const override;

    std::vector<domain::Alarm> findActive(
        std::size_t limit,
        const std::optional<std::string>& zoneId = std::nullopt
    ) const override;

    void create(const domain::Alarm& alarm) override;

    void resolve(
        const std::string& alarmId,
        std::chrono::system_clock::time_point resolvedAt
    ) override;

private:
    mutable mongocxx::collection alarmsCollection_;
};

}
