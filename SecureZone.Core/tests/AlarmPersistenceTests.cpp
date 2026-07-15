#include <cassert>
#include <chrono>
#include <optional>
#include <string>

#include "securezone/alarm/AlarmPersistenceService.h"

namespace {

using namespace securezone;

class FakeAlarmRepository final : public repository::IAlarmRepository {
public:
    std::optional<domain::Alarm> activeAlarm;
    int createCount{};
    int resolveCount{};

    std::optional<domain::Alarm> findActiveByTrackAndZone(
        const std::string& trackId,
        const std::string& zoneId
    ) const override {
        if (activeAlarm.has_value() && activeAlarm->trackId == trackId && activeAlarm->zoneId == zoneId) {
            return activeAlarm;
        }
        return std::nullopt;
    }

    void create(const domain::Alarm& alarm) override {
        activeAlarm = alarm;
        ++createCount;
    }

    void resolve(
        const std::string& alarmId,
        std::chrono::system_clock::time_point resolvedAt
    ) override {
        assert(activeAlarm.has_value());
        assert(activeAlarm->alarmId == alarmId);
        activeAlarm->status = domain::AlarmStatus::Resolved;
        activeAlarm->stillInside = false;
        activeAlarm->resolvedAt = resolvedAt;
        ++resolveCount;
    }
};

decision::DecisionContext makeContext(
    const domain::ZoneEntryEvent& zoneEntryEvent,
    const domain::Zone& zone,
    const domain::MachineState& machineState,
    const domain::AccessPolicy& accessPolicy
) {
    return {zoneEntryEvent, zone, std::nullopt, machineState, accessPolicy, true, false, false};
}

}

int main() {
    const auto timestamp = std::chrono::system_clock::now();
    const domain::ZoneEntryEvent zoneEntryEvent{"event-1", "track-1", "camera-1", "Camera 1", timestamp};
    const domain::Zone zone{"zone-1", "Danger zone", "camera-1", domain::ZoneType::Dangerous};
    const domain::MachineState machine{"machine-1", domain::MachineStatus::Stopped};
    const domain::AccessPolicy policy{"policy-1", "zone-1", {}, {domain::MachineStatus::Stopped}};
    const auto context = makeContext(zoneEntryEvent, zone, machine, policy);

    FakeAlarmRepository repository;
    alarm::AlarmPersistenceService service{repository};
    const domain::AccessDecision violation{
        domain::AccessDecisionType::Violation,
        "Employee role is not allowed in this zone.",
        true,
        false
    };

    assert(service.persist(violation, context, timestamp) == alarm::AlarmPersistenceAction::Created);
    assert(repository.createCount == 1);
    assert(repository.activeAlarm->reason == violation.reason);
    assert(repository.activeAlarm->status == domain::AlarmStatus::Active);
    assert(service.persist(violation, context, timestamp) == alarm::AlarmPersistenceAction::None);
    assert(repository.createCount == 1);

    const domain::AccessDecision clear{
        domain::AccessDecisionType::Allowed,
        "Person is outside the zone.",
        false,
        true
    };
    assert(service.persist(clear, context, timestamp) == alarm::AlarmPersistenceAction::Resolved);
    assert(repository.resolveCount == 1);
    assert(repository.activeAlarm->resolvedAt.has_value());
}
