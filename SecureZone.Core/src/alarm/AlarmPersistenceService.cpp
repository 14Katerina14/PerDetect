#include "securezone/alarm/AlarmPersistenceService.h"

#include <chrono>
#include <sstream>
#include <string>

namespace securezone::alarm {

namespace {

using Clock = std::chrono::system_clock;

std::string timestampSuffix(Clock::time_point timePoint) {
    const auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(
        timePoint.time_since_epoch()
    ).count();

    return std::to_string(milliseconds);
}

std::string createAlarmId(
    const std::string& trackId,
    const std::string& zoneId,
    Clock::time_point enteredAt
) {
    return "ALARM-" + trackId + "-" + zoneId + "-" + timestampSuffix(enteredAt);
}

std::string employeeIdFromContext(const decision::DecisionContext& context) {
    if (!context.employee.has_value()) {
        return {};
    }

    return context.employee->get().employeeId;
}

std::string createAlarmMessage(
    const domain::AccessDecision& decision,
    const decision::DecisionContext& context
) {
    std::ostringstream message;
    message << decision.reason
            << " Track "
            << context.zoneEntryEvent.trackId
            << " is inside zone "
            << context.zone.name
            << ".";

    if (context.employee.has_value()) {
        message << " Employee: " << context.employee->get().fullName << ".";
    }

    return message.str();
}

domain::Alarm createAlarm(
    const domain::AccessDecision& decision,
    const decision::DecisionContext& context
) {
    domain::Alarm alarm{};
    alarm.alarmId = createAlarmId(
        context.zoneEntryEvent.trackId,
        context.zone.zoneId,
        context.zoneEntryEvent.timestamp
    );
    alarm.zoneId = context.zone.zoneId;
    alarm.trackId = context.zoneEntryEvent.trackId;
    alarm.employeeId = employeeIdFromContext(context);
    alarm.machineId = context.machineState.machineId;
    alarm.status = domain::AlarmStatus::Active;
    alarm.reason = decision.reason;
    alarm.enteredAt = context.zoneEntryEvent.timestamp;
    alarm.stillInside = true;
    alarm.message = createAlarmMessage(decision, context);
    return alarm;
}

}

AlarmPersistenceService::AlarmPersistenceService(
    repository::IAlarmRepository& alarmRepository
) : alarmRepository_{alarmRepository} {
}

AlarmPersistenceAction AlarmPersistenceService::persist(
    const domain::AccessDecision& decision,
    const decision::DecisionContext& context,
    Clock::time_point handledAt
) {
    auto activeAlarm = alarmRepository_.findActiveByTrackAndZone(
        context.zoneEntryEvent.trackId,
        context.zone.zoneId
    );

    if (decision.shouldClearAlarm) {
        if (!activeAlarm.has_value()) {
            return AlarmPersistenceAction::None;
        }

        alarmRepository_.resolve(activeAlarm->alarmId, handledAt);
        return AlarmPersistenceAction::Resolved;
    }

    if (decision.shouldCreateAlarm) {
        if (activeAlarm.has_value()) {
            return AlarmPersistenceAction::None;
        }

        alarmRepository_.create(createAlarm(decision, context));
        return AlarmPersistenceAction::Created;
    }

    return AlarmPersistenceAction::None;
}

}
