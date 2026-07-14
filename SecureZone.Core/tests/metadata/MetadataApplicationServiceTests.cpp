#include "securezone/metadata/MetadataApplicationService.h"

#include <cassert>
#include <chrono>
#include <optional>
#include <string>
#include <vector>

namespace {

using namespace securezone;
using Clock = std::chrono::system_clock;

class FakeMetadataParser final : public metadata::IMetadataParser {
public:
    metadata::ParsedMetadataFrame parse(
        const std::string& cameraId,
        const std::string& rawMetadata
    ) const override {
        assert(cameraId == "CAM-001");
        assert(rawMetadata == "<metadata/>");

        const auto timestamp = Clock::time_point{std::chrono::seconds{30}};

        domain::Detection detection{};
        detection.trackId = "TRACK-001";
        detection.cameraId = cameraId;
        detection.objectClass = domain::ObjectClass::Person;
        detection.bbox = {10.0, 10.0, 20.0, 20.0};
        detection.confidence = 0.95;
        detection.timestamp = timestamp;

        domain::MetadataEvent metadataEvent{};
        metadataEvent.eventId = "EVENT-001";
        metadataEvent.cameraId = cameraId;
        metadataEvent.trackId = detection.trackId;
        metadataEvent.timestamp = timestamp;
        metadataEvent.objectClass = "Person";
        metadataEvent.bbox = detection.bbox;
        metadataEvent.eventType = "ObjectDetection";

        return metadata::ParsedMetadataFrame{
            cameraId,
            "WiseAI",
            timestamp,
            {detection},
            {metadataEvent}
        };
    }
};

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

class FakeCameraTrackRepository final : public repository::ICameraTrackRepository {
public:
    std::vector<domain::CameraTrack> upsertedTracks;

    std::optional<domain::CameraTrack> findByTrackId(
        const std::string&
    ) const override {
        return std::nullopt;
    }

    void upsert(const domain::CameraTrack& cameraTrack) override {
        upsertedTracks.push_back(cameraTrack);
    }

    void markLost(
        const std::string&,
        Clock::time_point
    ) override {
    }
};

class FakeMetadataEventRepository final : public repository::IMetadataEventRepository {
public:
    std::vector<domain::MetadataEvent> createdEvents;

    void create(const domain::MetadataEvent& metadataEvent) override {
        createdEvents.push_back(metadataEvent);
    }

    std::vector<domain::MetadataEvent> findRecentByTrackId(
        const std::string&,
        std::int64_t
    ) const override {
        return {};
    }
};

class FakeEmployeeRepository final : public repository::IEmployeeRepository {
public:
    std::optional<domain::Employee> findByEmployeeId(
        const std::string&
    ) const override {
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

    bool save(const domain::Zone& value) override {
        zone = value;
        return true;
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

    bool updateStatus(
        const std::string&,
        domain::MachineStatus,
        std::chrono::system_clock::time_point
    ) override {
        return true;
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

    bool save(const domain::AccessPolicy& value) override {
        accessPolicy = value;
        return true;
    }
};

class FakeTrackIdentityBindingRepository final
    : public repository::ITrackIdentityBindingRepository {
public:
    std::optional<domain::TrackIdentityBinding> findCurrentByTrackId(
        const std::string&
    ) const override {
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
        activeAlarm->resolvedAt = resolvedAt;
        activeAlarm->stillInside = false;
    }
};

void handlesRawMetadataThroughPersistenceAndDecisionTrigger() {
    const FakeMetadataParser parser{};
    metadata::MetadataIngestionService ingestionService{parser};

    FakeCameraTrackRepository cameraTrackRepository{};
    FakeMetadataEventRepository metadataEventRepository{};
    metadata::MetadataPersistenceService persistenceService{
        cameraTrackRepository,
        metadataEventRepository
    };
    metadata::MetadataProcessingService processingService{
        ingestionService,
        persistenceService
    };

    geometry::ZoneGeometryService zoneGeometryService{};
    FakeEmployeeRepository employeeRepository{};
    FakeZoneRepository zoneRepository{};
    FakeMachineRepository machineRepository{};
    FakeAccessPolicyRepository accessPolicyRepository{};
    decision::DecisionContextLoader contextLoader{
        employeeRepository,
        zoneRepository,
        machineRepository,
        accessPolicyRepository
    };
    decision::DecisionEngine decisionEngine{};
    FakeTrackIdentityBindingRepository bindingRepository{};
    FakeAlarmRepository alarmRepository{};
    alarm::AlarmPersistenceService alarmPersistenceService{alarmRepository};
    metadata::MetadataDecisionTriggerService decisionTriggerService{
        zoneGeometryService,
        contextLoader,
        decisionEngine,
        alarmPersistenceService,
        bindingRepository,
        alarmRepository
    };

    metadata::MetadataApplicationService applicationService{
        processingService,
        decisionTriggerService
    };

    const auto result = applicationService.handle(
        metadata::MetadataApplicationRequest{
            "CAM-001",
            "<metadata/>",
            {makeZone()},
            false
        }
    );

    assert(result.processing.detectionsProcessed == 1);
    assert(result.processing.tracksUpserted == 1);
    assert(result.processing.eventsCreated == 2);
    assert(result.decisions.detectionsChecked == 1);
    assert(result.decisions.decisionsEvaluated == 1);
    assert(result.decisions.violations == 1);
    assert(result.decisions.alarmsCreated == 1);

    assert(cameraTrackRepository.upsertedTracks.size() == 1);
    assert(metadataEventRepository.createdEvents.size() == 2);
    assert(alarmRepository.createdAlarms.size() == 1);
}

}

int main() {
    handlesRawMetadataThroughPersistenceAndDecisionTrigger();
}
