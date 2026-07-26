#include <cassert>
#include <chrono>
#include <optional>
#include <string>
#include <vector>

#include "securezone/repository/IAccessPolicyRepository.h"
#include "securezone/repository/IAlarmRepository.h"
#include "securezone/repository/IEmployeeRepository.h"
#include "securezone/repository/IMachineRepository.h"
#include "securezone/xprotect/XProtectPolicyAlarmService.h"

namespace {

using namespace securezone;
using Clock = std::chrono::system_clock;

class FakeEmployeeRepository final : public repository::IEmployeeRepository {
public:
    std::optional<domain::Employee> employee;

    std::optional<domain::Employee> findByEmployeeId(const std::string& employeeId) const override {
        return employee.has_value() && employee->employeeId == employeeId
            ? employee : std::nullopt;
    }
};

class FakeAccessPolicyRepository final : public repository::IAccessPolicyRepository {
public:
    std::optional<domain::AccessPolicy> policy;

    std::optional<domain::AccessPolicy> findByZoneId(const std::string& zoneId) const override {
        return policy.has_value() && policy->zoneId == zoneId ? policy : std::nullopt;
    }

    bool save(const domain::AccessPolicy& value) override {
        policy = value;
        return true;
    }
};

class FakeMachineRepository final : public repository::IMachineRepository {
public:
    std::optional<domain::MachineState> machine;

    std::optional<domain::MachineState> findByMachineId(const std::string& machineId) const override {
        return machine.has_value() && machine->machineId == machineId ? machine : std::nullopt;
    }

    bool updateStatus(
        const std::string& machineId,
        domain::MachineStatus status,
        Clock::time_point updatedAt
    ) override {
        if (!machine.has_value() || machine->machineId != machineId) return false;
        machine->status = status;
        machine->updatedAt = updatedAt;
        return true;
    }
};

class FakeAlarmRepository final : public repository::IAlarmRepository {
public:
    std::vector<domain::Alarm> alarms;
    int createCount{};
    int resolveCount{};

    std::optional<domain::Alarm> findActiveByTrackAndZone(
        const std::string& trackId,
        const std::string& zoneId
    ) const override {
        for (const auto& alarm : alarms) {
            if (alarm.trackId == trackId && alarm.zoneId == zoneId
                && (alarm.status == domain::AlarmStatus::Active
                    || alarm.status == domain::AlarmStatus::Acknowledged)) {
                return alarm;
            }
        }
        return std::nullopt;
    }

    std::size_t countActiveByZone(const std::string& zoneId) const override {
        std::size_t count{};
        for (const auto& alarm : alarms) {
            if (alarm.zoneId == zoneId
                && (alarm.status == domain::AlarmStatus::Active
                    || alarm.status == domain::AlarmStatus::Acknowledged)) ++count;
        }
        return count;
    }

    void create(const domain::Alarm& alarm) override {
        alarms.push_back(alarm);
        ++createCount;
    }

    void resolve(const std::string& alarmId, Clock::time_point resolvedAt) override {
        for (auto& alarm : alarms) {
            if (alarm.alarmId != alarmId) continue;
            alarm.status = domain::AlarmStatus::Resolved;
            alarm.stillInside = false;
            alarm.exitedAt = resolvedAt;
            alarm.resolvedAt = resolvedAt;
            ++resolveCount;
            return;
        }
    }
};

struct Fixture {
    Clock::time_point now{Clock::now()};
    FakeEmployeeRepository employees;
    FakeAccessPolicyRepository policies;
    FakeMachineRepository machines;
    FakeAlarmRepository alarms;
    domain::Zone zone;
    domain::TrackIdentityBinding binding;
    xprotect::XProtectLineCrossingCommand command;

    Fixture() {
        employees.employee = domain::Employee{
            "EMP-001", "Ivan Petrov", {"maintenance"},
            domain::EmployeeStatus::Active, "Maintenance", "hash"
        };
        policies.policy = domain::AccessPolicy{
            "POL-001", "ZONE-001", {"maintenance"},
            {domain::MachineStatus::Stopped}
        };
        machines.machine = domain::MachineState{
            "MACHINE-001", domain::MachineStatus::Stopped, "Machine A", now
        };
        zone.zoneId = "ZONE-001";
        zone.name = "Machine A dangerous zone";
        zone.cameraId = "CAM-001";
        zone.type = domain::ZoneType::Dangerous;
        zone.status = domain::ZoneStatus::Active;
        zone.relatedMachineId = "MACHINE-001";
        binding.bindingId = "BIND-001";
        binding.cameraId = "CAM-001";
        binding.objectId = "42";
        binding.employeeId = "EMP-001";
        binding.presenceSessionId = "SESSION-001";
        binding.boundAt = now - std::chrono::seconds{5};
        binding.expiresAt = now + std::chrono::minutes{5};
        binding.status = domain::TrackIdentityBindingStatus::Active;
        command = {
            "OpenSDK.WiseAI.LineCrossing.State-2",
            "Camera 1",
            now,
            "CAM-001",
            "42",
            "enter"
        };
    }

    xprotect::XProtectPolicyAlarmService service() {
        return {employees, policies, machines, alarms};
    }
};

domain::Alarm activeAlarm(const std::string& trackId, const std::string& alarmId) {
    domain::Alarm alarm{};
    alarm.alarmId = alarmId;
    alarm.zoneId = "ZONE-001";
    alarm.trackId = trackId;
    alarm.status = domain::AlarmStatus::Active;
    alarm.stillInside = true;
    return alarm;
}

void missingBindingCreatesViolation() {
    Fixture fixture;
    auto service = fixture.service();
    const auto result = service.evaluate(fixture.command, fixture.zone, std::nullopt);
    assert(result.decision == "violation");
    assert(fixture.alarms.createCount == 1);
}

void inactiveEmployeeCreatesViolation() {
    Fixture fixture;
    fixture.employees.employee->status = domain::EmployeeStatus::Inactive;
    auto service = fixture.service();
    const auto result = service.evaluate(fixture.command, fixture.zone, fixture.binding);
    assert(result.decision == "violation");
    assert(result.message == "Employee is inactive.");
}

void deniedRoleCreatesViolation() {
    Fixture fixture;
    fixture.employees.employee->roles = {"operator"};
    auto service = fixture.service();
    const auto result = service.evaluate(fixture.command, fixture.zone, fixture.binding);
    assert(result.decision == "violation");
    assert(result.message == "Employee role is not allowed in this zone.");
}

void deniedMachineStateCreatesViolation() {
    Fixture fixture;
    fixture.machines.machine->status = domain::MachineStatus::Running;
    auto service = fixture.service();
    const auto result = service.evaluate(fixture.command, fixture.zone, fixture.binding);
    assert(result.decision == "violation");
    assert(result.message == "Machine state does not allow access.");
}

void allowedEmployeeDoesNotCreateAlarm() {
    Fixture fixture;
    auto service = fixture.service();
    const auto result = service.evaluate(fixture.command, fixture.zone, fixture.binding);
    assert(result.decision == "allowed");
    assert(fixture.alarms.createCount == 0);
}

void repeatedEventDoesNotDuplicateAlarm() {
    Fixture fixture;
    fixture.employees.employee->roles = {"operator"};
    auto service = fixture.service();
    const auto first = service.evaluate(fixture.command, fixture.zone, fixture.binding);
    const auto second = service.evaluate(fixture.command, fixture.zone, fixture.binding);
    assert(first.decision == "violation");
    assert(second.decision == "violation_active");
    assert(fixture.alarms.createCount == 1);
}

void notifiesOnlyWhenANewAlarmIsCreated() {
    Fixture fixture;
    fixture.employees.employee->roles = {"operator"};
    std::vector<std::string> notifiedAlarmIds;
    xprotect::XProtectPolicyAlarmService service{
        fixture.employees,
        fixture.policies,
        fixture.machines,
        fixture.alarms,
        [&notifiedAlarmIds](const domain::Alarm& alarm) {
            notifiedAlarmIds.push_back(alarm.alarmId);
        }
    };

    service.evaluate(fixture.command, fixture.zone, fixture.binding);
    service.evaluate(fixture.command, fixture.zone, fixture.binding);

    assert(notifiedAlarmIds.size() == 1U);
    assert(notifiedAlarmIds.front() == fixture.alarms.alarms.front().alarmId);
}

void lastViolatorExitClearsZone() {
    Fixture fixture;
    fixture.alarms.alarms.push_back(activeAlarm("CAM-001:42", "ALARM-001"));
    fixture.command.action = "exit";
    auto service = fixture.service();
    const auto result = service.evaluate(fixture.command, fixture.zone, fixture.binding);
    assert(result.decision == "cleared");
    assert(fixture.alarms.resolveCount == 1);
    assert(fixture.alarms.countActiveByZone("ZONE-001") == 0);
}

void lastViolatorExitNotifiesWithResolvedAlarm() {
    Fixture fixture;
    fixture.alarms.alarms.push_back(activeAlarm("CAM-001:42", "ALARM-001"));
    fixture.command.action = "exit";
    std::vector<domain::Alarm> resolvedNotifications;
    xprotect::XProtectPolicyAlarmService service{
        fixture.employees,
        fixture.policies,
        fixture.machines,
        fixture.alarms,
        {},
        [&resolvedNotifications](const domain::Alarm& alarm) {
            resolvedNotifications.push_back(alarm);
        }
    };

    const auto result = service.evaluate(fixture.command, fixture.zone, fixture.binding);

    assert(result.decision == "cleared");
    assert(resolvedNotifications.size() == 1U);
    assert(resolvedNotifications.front().alarmId == "ALARM-001");
    assert(resolvedNotifications.front().status == domain::AlarmStatus::Resolved);
    assert(!resolvedNotifications.front().stillInside);
    assert(resolvedNotifications.front().resolvedAt == fixture.command.receivedAt);
}

void exitKeepsAlarmActiveWhenAnotherViolatorRemains() {
    Fixture fixture;
    fixture.alarms.alarms.push_back(activeAlarm("CAM-001:42", "ALARM-001"));
    fixture.alarms.alarms.push_back(activeAlarm("CAM-001:99", "ALARM-002"));
    fixture.command.action = "outbound";
    auto service = fixture.service();
    const auto result = service.evaluate(fixture.command, fixture.zone, fixture.binding);
    assert(result.decision == "violation_active");
    assert(fixture.alarms.resolveCount == 1);
    assert(fixture.alarms.countActiveByZone("ZONE-001") == 1);
}

}

int main() {
    missingBindingCreatesViolation();
    inactiveEmployeeCreatesViolation();
    deniedRoleCreatesViolation();
    deniedMachineStateCreatesViolation();
    allowedEmployeeDoesNotCreateAlarm();
    repeatedEventDoesNotDuplicateAlarm();
    notifiesOnlyWhenANewAlarmIsCreated();
    lastViolatorExitClearsZone();
    lastViolatorExitNotifiesWithResolvedAlarm();
    exitKeepsAlarmActiveWhenAnotherViolatorRemains();
}
