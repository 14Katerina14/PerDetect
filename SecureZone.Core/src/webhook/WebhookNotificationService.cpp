#include "securezone/webhook/WebhookNotificationService.h"

#include <optional>
#include <string>

namespace securezone::webhook {

namespace {

std::string alarmStatusToString(domain::AlarmStatus status) {
    switch (status) {
        case domain::AlarmStatus::Created:
            return "created";
        case domain::AlarmStatus::Active:
            return "active";
        case domain::AlarmStatus::Acknowledged:
            return "acknowledged";
        case domain::AlarmStatus::Resolved:
            return "resolved";
    }

    return "created";
}

std::string machineStatusToString(domain::MachineStatus status) {
    switch (status) {
        case domain::MachineStatus::Running:
            return "running";
        case domain::MachineStatus::Stopped:
            return "stopped";
        case domain::MachineStatus::Maintenance:
            return "maintenance";
    }

    return "stopped";
}

std::optional<std::chrono::system_clock::time_point> optionalTimestamp(
    std::chrono::system_clock::time_point value
) {
    if (value == std::chrono::system_clock::time_point{}) {
        return std::nullopt;
    }

    return value;
}

}

AlarmNotificationPayload WebhookNotificationService::createAlarmNotificationPayload(
    const domain::Alarm& alarm,
    const AlarmNotificationContext& context
) const {
    AlarmNotificationPayload payload{};
    payload.alarmId = alarm.alarmId;
    payload.status = alarmStatusToString(alarm.status);
    payload.reason = alarm.reason;
    payload.zoneId = context.zone.zoneId;
    payload.zoneName = context.zone.name;
    payload.trackId = alarm.trackId;
    payload.enteredAt = alarm.enteredAt;
    payload.exitedAt = optionalTimestamp(alarm.exitedAt);
    payload.resolvedAt = alarm.resolvedAt;
    payload.stillInside = alarm.stillInside;
    payload.message = alarm.message;
    payload.timestamp = context.timestamp;

    if (!alarm.employeeId.empty()) {
        payload.employeeId = alarm.employeeId;
    }

    if (context.employee.has_value()) {
        const auto& employee = context.employee->get();
        payload.employeeId = employee.employeeId;
        payload.employeeName = employee.fullName;
        payload.employeeRoles = employee.roles;
    }

    if (!alarm.machineId.empty()) {
        payload.machineId = alarm.machineId;
    }

    if (context.machine.has_value()) {
        const auto& machine = context.machine->get();
        payload.machineId = machine.machineId;
        payload.machineName = machine.name;
        payload.machineStatus = machineStatusToString(machine.status);
    }

    return payload;
}

}
