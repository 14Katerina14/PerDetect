#pragma once

#include <vector>

#include "securezone/alarm/AlarmPersistenceService.h"
#include "securezone/decision/DecisionContextLoader.h"
#include "securezone/decision/DecisionEngine.h"
#include "securezone/domain/Zone.h"
#include "securezone/geometry/ZoneGeometryService.h"
#include "securezone/metadata/MetadataDecisionTriggerResult.h"
#include "securezone/metadata/MetadataIngestionResult.h"
#include "securezone/repository/IAlarmRepository.h"
#include "securezone/repository/ITrackIdentityBindingRepository.h"

namespace securezone::metadata {

struct MetadataDecisionTriggerRequest {
    MetadataIngestionResult ingestionResult;
    std::vector<domain::Zone> candidateZones;
    bool isIdentityGracePeriodActive{};
};

class MetadataDecisionTriggerService {
public:
    MetadataDecisionTriggerService(
        const geometry::ZoneGeometryService& zoneGeometryService,
        const decision::DecisionContextLoader& decisionContextLoader,
        const decision::DecisionEngine& decisionEngine,
        alarm::AlarmPersistenceService& alarmPersistenceService,
        const repository::ITrackIdentityBindingRepository& trackIdentityBindingRepository,
        const repository::IAlarmRepository& alarmRepository
    );

    MetadataDecisionTriggerResult trigger(const MetadataDecisionTriggerRequest& request);

private:
    const geometry::ZoneGeometryService& zoneGeometryService_;
    const decision::DecisionContextLoader& decisionContextLoader_;
    const decision::DecisionEngine& decisionEngine_;
    alarm::AlarmPersistenceService& alarmPersistenceService_;
    const repository::ITrackIdentityBindingRepository& trackIdentityBindingRepository_;
    const repository::IAlarmRepository& alarmRepository_;
};

}
