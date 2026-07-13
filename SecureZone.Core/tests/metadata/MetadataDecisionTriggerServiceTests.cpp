#include "securezone/metadata/MetadataDecisionTriggerService.h"

#include <cassert>
#include <chrono>
#include <optional>
#include <string>
#include <vector>

namespace {

using namespace securezone;
using Clock = std::chrono::system_clock;

domain::Detection makeDetection(
    const domain::BoundingBox& bbox = {10.0, 10.0, 20.0, 20.0}
) {
    domain::Detection detection{};
    detection.trackId = "TRACK-001";
    detection.cameraId = "CAM-001";
    detection.objectClass = domain::ObjectClass::Person;
    detection.bbox = bbox;
    detection.confidence = 0.95;
    detection.timestamp = Clock::time_point{std::chrono::seconds{30}};
    return detection;
}

domain::Zone makeZone() {
    domain::Zone zone{};
    zone.zoneId = "ZONE-001";
    zone.name = "Machine A Dangerous Zone";
    zone.cameraId = "CAM-001";
    zone.type = domain::ZoneType::Dangerous;
    zone.status = domain::ZoneStatus::Active;
    zone.polygon = {{0.0, 0.0}, {100.0, 0.0}, {100.0, 100.0}, {0.0, 100.0}};
    zone.relatedMachineId = "MACHINE-001";
    return zone;
}

domain::MachineState makeMachineState() {
    domain::MachineState machineState{};
    machineState.machineId = "MACHINE-001";
    machineState.status = domain::MachineStatus::Running;
    machineState.name = "Machine A";
    return machineState;
}

domain::AccessPolicy makeAccessPolicy() {
    domain::AccessPolicy accessPolicy{};
    accessPolicy.policyId = "POLICY-001";
    accessPolicy.zoneId = "ZONE-001";
    accessPolicy.allowedRoles = {"maintenance"};
    accessPolicy.machineStatesAllowed = {domain::MachineStatus::Stopped};
    return accessPolicy;
}

metadata::MetadataIngestionResult makeIngestionResult(const domain::Detection& detection) {
    metadata::MetadataIngestionResult ingestionResult{};
    ingestionResult.detections.push_back(detection);
    return ingestionResult;
}

class FakeEmployeeRepository final : public repository::IEmployeeRepository {
public:
    std::optional<domain::Employee> employee;

    std::optional<domain::Employee> findByEmployeeId(
        const std::string& employeeId
    ) const override {
        if (employee.has_value() && employee->employeeId == employeeId) {
            return employee;
        }

        return std::nullopt;
    }
};

class FakeZoneRepository final : public repository::IZoneRepository {
public:
    domain::Zone zone = makeZone();

    std::optional<domain::Zone> findByZoneId(
        const std::string& zoneId
    ) const override {
        if (zone.zoneId == zoneId) {
            return zone;
        }

        return std::nullopt;
    }

    std::optional<domain::Zone> findActiveByZoneId(
        const std::string& zoneId
    ) const override {
        if (zone.zoneId == zoneId && zone.status == domain::ZoneStatus::Active) {
            return zone;
        }

        return std::nullopt;
    }
};

class FakeMachineRepository final : public repository::IMachineRepository {
public:
    domain::MachineState machineState = makeMachineState();

    std::optional<domain::MachineState> findByMachineId(
        const std::string& machineId
    ) const override {
        if (machineState.machineId == machineId) {
            return machineState;
        }

        return std::nullopt;
    }
};

class FakeAccessPolicyRepository final : public repository::IAccessPolicyRepository {
public:
    domain::AccessPolicy accessPolicy = makeAccessPolicy();

    std::optional<domain::AccessPolicy> findByZoneId(
        const std::string& zoneId
    ) const override {
        if (accessPolicy.zoneId == zoneId) {
            return accessPolicy;
        }

        return std::nullopt;
    }
};

class FakeTrackIdentityBindingRepository final
    : public repository::ITrackIdentityBindingRepository {
public:
    std::optional<domain::TrackIdentityBinding> binding;

    std::optional<domain::TrackIdentityBinding> findCurrentByTrackId(
        const std::string& trackId
    ) const override {
        if (binding.has_value() && binding->trackId == trackId) {
            return binding;
        }

        return std::nullopt;
    }

    void create(const domain::TrackIdentityBinding&) override {
    }

    void updateStatus(
        const std::string&,
        const std::string&
    ) override {
    }
};

class FakeAlarmRepository final : public repository::IAlarmRepository {
public:
    std::optional<domain::Alarm> activeAlarm;
    std::vector<domain::Alarm> createdAlarms;
    std::vector<std::string> resolvedAlarmIds;

    std::optional<domain::Alarm> findActiveByTrackAndZone(
        const std::string& trackId,
        const std::string& zoneId
    ) const override {
        if (activeAlarm.has_value()
            && activeAlarm->trackId == trackId
            && activeAlarm->zoneId == zoneId
            && activeAlarm->status != domain::AlarmStatus::Resolved) {
            return activeAlarm;
        }

        return std::nullopt;
    }

    void create(const domain::Alarm& alarm) override {
        createdAlarms.push_back(alarm);
        activeAlarm = alarm;
    }

    void resolve(
        const std::string& alarmId,
        Clock::time_point resolvedAt
    ) override {
        assert(activeAlarm.has_value());
        assert(activeAlarm->alarmId == alarmId);

        activeAlarm->status = domain::AlarmStatus::Resolved;
        activeAlarm->stillInside = false;
        activeAlarm->resolvedAt = resolvedAt;
        resolvedAlarmIds.push_back(alarmId);
    }
};

struct TestContext {
    geometry::ZoneGeometryService zoneGeometryService;
    FakeEmployeeRepository employeeRepository;
    FakeZoneRepository zoneRepository;
    FakeMachineRepository machineRepository;
    FakeAccessPolicyRepository accessPolicyRepository;
    FakeTrackIdentityBindingRepository bindingRepository;
    FakeAlarmRepository alarmRepository;
    decision::DecisionContextLoader contextLoader{
        employeeRepository,
        zoneRepository,
        machineRepository,
        accessPolicyRepository
    };
    decision::DecisionEngine decisionEngine;
    alarm::AlarmPersistenceService alarmPersistenceService{alarmRepository};
    metadata::MetadataDecisionTriggerService service{
        zoneGeometryService,
        contextLoader,
        decisionEngine,
        alarmPersistenceService,
        bindingRepository,
        alarmRepository
    };
};

void createsAlarmForUnknownPersonInsideDangerousZone() {
    TestContext context{};

    metadata::MetadataDecisionTriggerRequest request{};
    request.ingestionResult = makeIngestionResult(makeDetection());
    request.candidateZones = {makeZone()};
    request.isIdentityGracePeriodActive = false;

    const auto result = context.service.trigger(request);

    assert(result.detectionsChecked == 1);
    assert(result.decisionsEvaluated == 1);
    assert(result.violations == 1);
    assert(result.alarmsCreated == 1);
    assert(context.alarmRepository.createdAlarms.size() == 1);
    assert(context.alarmRepository.createdAlarms.front().trackId == "TRACK-001");
    assert(context.alarmRepository.createdAlarms.front().zoneId == "ZONE-001");
}

void resolvesActiveAlarmWhenPersonLeavesZone() {
    TestContext context{};

    domain::Alarm activeAlarm{};
    activeAlarm.alarmId = "ALARM-001";
    activeAlarm.trackId = "TRACK-001";
    activeAlarm.zoneId = "ZONE-001";
    activeAlarm.status = domain::AlarmStatus::Active;
    context.alarmRepository.activeAlarm = activeAlarm;

    metadata::MetadataDecisionTriggerRequest request{};
    request.ingestionResult = makeIngestionResult(makeDetection({200.0, 200.0, 20.0, 20.0}));
    request.candidateZones = {makeZone()};
    request.isIdentityGracePeriodActive = false;

    const auto result = context.service.trigger(request);

    assert(result.detectionsChecked == 1);
    assert(result.decisionsEvaluated == 1);
    assert(result.allowed == 1);
    assert(result.alarmsResolved == 1);
    assert(context.alarmRepository.resolvedAlarmIds.size() == 1);
    assert(context.alarmRepository.resolvedAlarmIds.front() == "ALARM-001");
}

}

int main() {
    createsAlarmForUnknownPersonInsideDangerousZone();
    resolvesActiveAlarmWhenPersonLeavesZone();
}
