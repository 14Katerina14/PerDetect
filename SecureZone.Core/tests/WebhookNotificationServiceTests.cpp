#include "securezone/webhook/WebhookNotificationService.h"

#include <cassert>
#include <chrono>
#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace {

using namespace securezone;
using Clock = std::chrono::system_clock;

Clock::time_point at(int seconds) {
    return Clock::time_point{} + std::chrono::seconds{seconds};
}

domain::Alarm makeAlarm() {
    domain::Alarm alarm{};
    alarm.alarmId = "ALARM-001";
    alarm.zoneId = "ZONE-001";
    alarm.trackId = "TRACK-001";
    alarm.employeeId = "EMP-001";
    alarm.machineId = "MACHINE-001";
    alarm.status = domain::AlarmStatus::Active;
    alarm.reason = "Employee role is not allowed in this zone.";
    alarm.message = "Unauthorized presence in dangerous zone.";
    alarm.enteredAt = at(10);
    alarm.stillInside = true;
    return alarm;
}

domain::Zone makeZone() {
    domain::Zone zone{};
    zone.zoneId = "ZONE-001";
    zone.name = "Dangerous Zone";
    return zone;
}

domain::Employee makeEmployee() {
    domain::Employee employee{};
    employee.employeeId = "EMP-001";
    employee.fullName = "Ivan Petrov";
    employee.roles = {"maintenance", "supervisor"};
    return employee;
}

domain::MachineState makeMachine() {
    domain::MachineState machine{};
    machine.machineId = "MACHINE-001";
    machine.name = "Machine A";
    machine.status = domain::MachineStatus::Running;
    return machine;
}

webhook::AlarmNotificationContext makeContext(
    const domain::Zone& zone,
    std::optional<std::reference_wrapper<const domain::Employee>> employee = std::nullopt,
    std::optional<std::reference_wrapper<const domain::MachineState>> machine = std::nullopt
) {
    return {zone, employee, machine, at(20)};
}

void createsCompletePayloadForIdentifiedEmployeeAndMachine() {
    const auto alarm = makeAlarm();
    const auto zone = makeZone();
    const auto employee = makeEmployee();
    const auto machine = makeMachine();

    const auto payload = webhook::WebhookNotificationService{}.createAlarmNotificationPayload(
        alarm,
        makeContext(zone, std::cref(employee), std::cref(machine))
    );

    assert(payload.alarmId == "ALARM-001");
    assert(payload.status == "active");
    assert(payload.zoneId == "ZONE-001");
    assert(payload.zoneName == "Dangerous Zone");
    assert(payload.trackId == "TRACK-001");
    assert(payload.employeeId == "EMP-001");
    assert(payload.employeeName == "Ivan Petrov");
    assert((payload.employeeRoles == std::vector<std::string>{"maintenance", "supervisor"}));
    assert(payload.machineId == "MACHINE-001");
    assert(payload.machineName == "Machine A");
    assert(payload.machineStatus == "running");
    assert(payload.enteredAt == at(10));
    assert(!payload.exitedAt.has_value());
    assert(payload.stillInside);
    assert(payload.timestamp == at(20));
}

void preservesKnownEmployeeIdWithoutEmployeeDetails() {
    const auto alarm = makeAlarm();
    const auto zone = makeZone();

    const auto payload = webhook::WebhookNotificationService{}.createAlarmNotificationPayload(
        alarm,
        makeContext(zone)
    );

    assert(payload.employeeId == "EMP-001");
    assert(!payload.employeeName.has_value());
    assert(payload.employeeRoles.empty());
}

void leavesEmployeeFieldsEmptyForUnknownIdentity() {
    auto alarm = makeAlarm();
    alarm.employeeId.clear();
    const auto zone = makeZone();

    const auto payload = webhook::WebhookNotificationService{}.createAlarmNotificationPayload(
        alarm,
        makeContext(zone)
    );

    assert(!payload.employeeId.has_value());
    assert(!payload.employeeName.has_value());
    assert(payload.employeeRoles.empty());
}

void preservesKnownMachineIdWithoutMachineDetails() {
    const auto alarm = makeAlarm();
    const auto zone = makeZone();

    const auto payload = webhook::WebhookNotificationService{}.createAlarmNotificationPayload(
        alarm,
        makeContext(zone)
    );

    assert(payload.machineId == "MACHINE-001");
    assert(!payload.machineName.has_value());
    assert(!payload.machineStatus.has_value());
}

void leavesMachineFieldsEmptyWhenNoMachineIsLinked() {
    auto alarm = makeAlarm();
    alarm.machineId.clear();
    const auto zone = makeZone();

    const auto payload = webhook::WebhookNotificationService{}.createAlarmNotificationPayload(
        alarm,
        makeContext(zone)
    );

    assert(!payload.machineId.has_value());
    assert(!payload.machineName.has_value());
    assert(!payload.machineStatus.has_value());
}

void representsResolvedAlarm() {
    auto alarm = makeAlarm();
    alarm.status = domain::AlarmStatus::Resolved;
    alarm.stillInside = false;
    alarm.exitedAt = at(30);
    alarm.resolvedAt = at(31);
    const auto zone = makeZone();

    const auto payload = webhook::WebhookNotificationService{}.createAlarmNotificationPayload(
        alarm,
        makeContext(zone)
    );

    assert(payload.status == "resolved");
    assert(payload.exitedAt == at(30));
    assert(payload.resolvedAt == at(31));
    assert(!payload.stillInside);
}

}

int main() {
    createsCompletePayloadForIdentifiedEmployeeAndMachine();
    preservesKnownEmployeeIdWithoutEmployeeDetails();
    leavesEmployeeFieldsEmptyForUnknownIdentity();
    preservesKnownMachineIdWithoutMachineDetails();
    leavesMachineFieldsEmptyWhenNoMachineIsLinked();
    representsResolvedAlarm();
}
