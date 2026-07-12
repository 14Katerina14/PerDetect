#pragma once

#include <chrono>
#include <string>

#include "securezone/decision/DecisionContext.h"
#include "securezone/domain/AccessDecision.h"
#include "securezone/repository/IAlarmRepository.h"

namespace securezone::alarm {

enum class AlarmPersistenceAction {
    None,
    Created,
    Resolved
};

class AlarmPersistenceService {
public:
    explicit AlarmPersistenceService(repository::IAlarmRepository& alarmRepository);

    AlarmPersistenceAction persist(
        const domain::AccessDecision& decision,
        const decision::DecisionContext& context,
        std::chrono::system_clock::time_point handledAt
    );

private:
    repository::IAlarmRepository& alarmRepository_;
};

}
