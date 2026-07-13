#pragma once

#include <chrono>
#include <functional>
#include <optional>
#include <string>
#include <vector>

#include "securezone/domain/Alarm.h"
#include "securezone/domain/Employee.h"
#include "securezone/domain/MachineState.h"
#include "securezone/domain/Zone.h"

namespace securezone::webhook {

struct AlarmNotificationContext {
    const domain::Zone& zone;
    std::optional<std::reference_wrapper<const domain::Employee>> employee;
    std::optional<std::reference_wrapper<const domain::MachineState>> machine;
    std::chrono::system_clock::time_point timestamp;
};

struct AlarmNotificationPayload {
    std::string alarmId;
    std::string status;
    std::string reason;
    std::string zoneId;
    std::string zoneName;
    std::string trackId;
    std::optional<std::string> employeeId;
    std::optional<std::string> employeeName;
    std::vector<std::string> employeeRoles;
    std::optional<std::string> machineId;
    std::optional<std::string> machineName;
    std::optional<std::string> machineStatus;
    std::chrono::system_clock::time_point enteredAt{};
    std::optional<std::chrono::system_clock::time_point> exitedAt;
    std::optional<std::chrono::system_clock::time_point> resolvedAt;
    bool stillInside{};
    std::string message;
    std::chrono::system_clock::time_point timestamp{};
};

class WebhookNotificationService {
public:
    AlarmNotificationPayload createAlarmNotificationPayload(
        const domain::Alarm& alarm,
        const AlarmNotificationContext& context
    ) const;
};

}
