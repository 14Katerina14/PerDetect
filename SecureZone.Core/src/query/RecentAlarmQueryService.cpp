#include "securezone/query/RecentAlarmQueryService.h"

#include <algorithm>
#include <utility>

namespace securezone::query {
namespace {

std::size_t normalizedLimit(std::size_t limit) {
    return std::min<std::size_t>(limit == 0U ? 50U : limit, 100U);
}

}

RecentAlarmQueryService::RecentAlarmQueryService(
    repository::IAlarmReadRepository& alarmRepository,
    repository::IEmployeeRepository& employeeRepository,
    repository::IZoneRepository& zoneRepository,
    repository::IMachineRepository& machineRepository
) : alarmRepository_{alarmRepository},
    employeeRepository_{employeeRepository},
    zoneRepository_{zoneRepository},
    machineRepository_{machineRepository} {
}

std::vector<AlarmView> RecentAlarmQueryService::list(
    std::size_t limit,
    const std::optional<std::string>& zoneId
) const {
    std::vector<AlarmView> views;
    for (const auto& alarm : alarmRepository_.findRecent(normalizedLimit(limit), zoneId)) {
        AlarmView view{};
        view.alarmId = alarm.alarmId;
        view.zoneId = alarm.zoneId;
        view.trackId = alarm.trackId;
        view.employeeId = alarm.employeeId;
        view.machineId = alarm.machineId;
        view.status = alarm.status;
        view.reason = alarm.reason;
        view.message = alarm.message;
        view.enteredAt = alarm.enteredAt;
        view.exitedAt = alarm.exitedAt;
        view.resolvedAt = alarm.resolvedAt;
        view.stillInside = alarm.stillInside;

        if (!alarm.zoneId.empty()) {
            const auto zone = zoneRepository_.findByZoneId(alarm.zoneId);
            if (zone.has_value()) view.zoneName = zone->name;
        }
        if (!alarm.employeeId.empty()) {
            const auto employee = employeeRepository_.findByEmployeeId(alarm.employeeId);
            if (employee.has_value()) view.employeeName = employee->fullName;
        }
        if (!alarm.machineId.empty()) {
            const auto machine = machineRepository_.findByMachineId(alarm.machineId);
            if (machine.has_value()) {
                view.machineName = machine->name;
                view.machineStatus = machine->status;
            }
        }
        views.push_back(std::move(view));
    }
    return views;
}

}
