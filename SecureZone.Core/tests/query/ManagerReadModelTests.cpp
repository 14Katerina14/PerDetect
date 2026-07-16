#include "securezone/query/ActiveAlarmQueryService.h"
#include "securezone/query/ActivePresenceQueryService.h"
#include "securezone/query/ZoneStatusQueryService.h"

#include <cassert>
#include <chrono>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace {

using namespace securezone;
using Clock = std::chrono::system_clock;

class FakeEmployeeRepository final : public repository::IEmployeeRepository {
public:
    std::vector<domain::Employee> employees;

    std::optional<domain::Employee> findByEmployeeId(const std::string& employeeId) const override {
        for (const auto& employee : employees) {
            if (employee.employeeId == employeeId) return employee;
        }
        return std::nullopt;
    }
};

class FakeZoneRepository final
    : public repository::IZoneRepository,
      public repository::IZoneStatusReadRepository {
public:
    std::vector<domain::Zone> zones;
    mutable std::size_t receivedLimit{};
    mutable std::optional<std::string> receivedCameraId;

    std::optional<domain::Zone> findByZoneId(const std::string& zoneId) const override {
        for (const auto& zone : zones) {
            if (zone.zoneId == zoneId) return zone;
        }
        return std::nullopt;
    }

    std::optional<domain::Zone> findActiveByZoneId(const std::string& zoneId) const override {
        const auto zone = findByZoneId(zoneId);
        return zone.has_value() && zone->status == domain::ZoneStatus::Active
            ? zone : std::nullopt;
    }

    bool save(const domain::Zone&) override { return true; }

    std::vector<domain::Zone> findAll(
        std::size_t limit,
        const std::optional<std::string>& cameraId
    ) const override {
        receivedLimit = limit;
        receivedCameraId = cameraId;
        return zones;
    }
};

class FakeMachineRepository final : public repository::IMachineRepository {
public:
    std::vector<domain::MachineState> machines;

    std::optional<domain::MachineState> findByMachineId(
        const std::string& machineId
    ) const override {
        for (const auto& machine : machines) {
            if (machine.machineId == machineId) return machine;
        }
        return std::nullopt;
    }

    bool updateStatus(
        const std::string&,
        domain::MachineStatus,
        Clock::time_point
    ) override {
        return false;
    }
};

class FakeAlarmReadRepository final : public repository::IAlarmReadRepository {
public:
    std::vector<domain::Alarm> alarms;
    mutable std::size_t receivedLimit{};
    mutable std::optional<std::string> receivedZoneId;

    std::vector<domain::Alarm> findActive(
        std::size_t limit,
        const std::optional<std::string>& zoneId
    ) const override {
        receivedLimit = limit;
        receivedZoneId = zoneId;
        return alarms;
    }
};

class FakePresenceReadRepository final : public repository::IPresenceSessionReadRepository {
public:
    std::vector<domain::PresenceSession> sessions;
    mutable Clock::time_point receivedAt{};
    mutable std::size_t receivedLimit{};
    mutable std::optional<std::string> receivedZoneId;

    std::vector<domain::PresenceSession> findActiveAt(
        Clock::time_point at,
        std::size_t limit,
        const std::optional<std::string>& zoneId
    ) const override {
        receivedAt = at;
        receivedLimit = limit;
        receivedZoneId = zoneId;
        return sessions;
    }
};

domain::Employee employee() {
    return {
        "EMP-001",
        "Ivan Petrov",
        {"maintenance"},
        domain::EmployeeStatus::Active,
        "Maintenance",
        {}
    };
}

domain::Zone zone(std::string machineId = "MACHINE-001") {
    domain::Zone value{};
    value.zoneId = "ZONE-001";
    value.name = "Machine A dangerous zone";
    value.cameraId = "CAM-001";
    value.type = domain::ZoneType::Dangerous;
    value.status = domain::ZoneStatus::Active;
    value.relatedMachineId = std::move(machineId);
    return value;
}

domain::MachineState machine() {
    return {
        "MACHINE-001",
        domain::MachineStatus::Running,
        "Machine A",
        {}
    };
}

void activeAlarmsAreEnrichedAndFiltered() {
    FakeAlarmReadRepository alarms;
    domain::Alarm alarm{};
    alarm.alarmId = "ALARM-001";
    alarm.zoneId = "ZONE-001";
    alarm.trackId = "CAM-001:42";
    alarm.employeeId = "EMP-001";
    alarm.machineId = "MACHINE-001";
    alarm.reason = "Role is not allowed.";
    alarm.message = "Employee entered a dangerous zone.";
    alarm.stillInside = true;
    alarms.alarms = {alarm};
    FakeEmployeeRepository employees;
    employees.employees = {employee()};
    FakeZoneRepository zones;
    zones.zones = {zone()};
    FakeMachineRepository machines;
    machines.machines = {machine()};
    query::ActiveAlarmQueryService service{alarms, employees, zones, machines};

    const auto result = service.list(500, std::string{"ZONE-001"});

    assert(alarms.receivedLimit == 100U);
    assert(alarms.receivedZoneId == "ZONE-001");
    assert(result.size() == 1U);
    assert(result.front().employeeName == "Ivan Petrov");
    assert(result.front().zoneName == "Machine A dangerous zone");
    assert(result.front().machineName == "Machine A");
    assert(result.front().machineStatus == domain::MachineStatus::Running);
    assert(result.front().stillInside);
}

void alarmsRemainVisibleWhenRelatedRecordsAreMissing() {
    FakeAlarmReadRepository alarms;
    domain::Alarm alarm{};
    alarm.alarmId = "ALARM-ORPHAN";
    alarm.zoneId = "MISSING-ZONE";
    alarm.employeeId = "MISSING-EMPLOYEE";
    alarm.machineId = "MISSING-MACHINE";
    alarms.alarms = {alarm};
    FakeEmployeeRepository employees;
    FakeZoneRepository zones;
    FakeMachineRepository machines;
    query::ActiveAlarmQueryService service{alarms, employees, zones, machines};

    const auto result = service.list(0);

    assert(alarms.receivedLimit == 50U);
    assert(result.size() == 1U);
    assert(result.front().employeeId == "MISSING-EMPLOYEE");
    assert(result.front().employeeName.empty());
    assert(!result.front().machineStatus.has_value());
}

void activePresenceIsEnrichedAtTheRequestedTime() {
    const auto now = Clock::time_point{} + std::chrono::seconds{100};
    FakePresenceReadRepository sessions;
    domain::PresenceSession session{};
    session.sessionId = "SESSION-001";
    session.employeeId = "EMP-001";
    session.zoneId = "ZONE-001";
    session.startedAt = now - std::chrono::seconds{10};
    session.expiresAt = now + std::chrono::seconds{10};
    sessions.sessions = {session};
    FakeEmployeeRepository employees;
    employees.employees = {employee()};
    FakeZoneRepository zones;
    zones.zones = {zone()};
    query::ActivePresenceQueryService service{sessions, employees, zones};

    const auto result = service.list(now, 20, std::string{"ZONE-001"});

    assert(sessions.receivedAt == now);
    assert(sessions.receivedLimit == 20U);
    assert(sessions.receivedZoneId == "ZONE-001");
    assert(result.size() == 1U);
    assert(result.front().employeeName == "Ivan Petrov");
    assert(result.front().zoneName == "Machine A dangerous zone");
}

void zonesAreEnrichedAndCameraFilterIsForwarded() {
    FakeZoneRepository zones;
    zones.zones = {zone(), zone("MISSING-MACHINE")};
    zones.zones.back().zoneId = "ZONE-002";
    FakeMachineRepository machines;
    machines.machines = {machine()};
    query::ZoneStatusQueryService service{zones, machines};

    const auto result = service.list(10, std::string{"CAM-001"});

    assert(zones.receivedLimit == 10U);
    assert(zones.receivedCameraId == "CAM-001");
    assert(result.size() == 2U);
    assert(result.front().machineStatus == domain::MachineStatus::Running);
    assert(!result.back().machineStatus.has_value());
}

}

int main() {
    activeAlarmsAreEnrichedAndFiltered();
    alarmsRemainVisibleWhenRelatedRecordsAreMissing();
    activePresenceIsEnrichedAtTheRequestedTime();
    zonesAreEnrichedAndCameraFilterIsForwarded();
}
