#include "securezone/metadata/MetadataDecisionTriggerService.h"

#include <optional>
#include <string>

namespace securezone::metadata {

namespace {

std::optional<std::string> boundEmployeeId(
    const repository::ITrackIdentityBindingRepository& repository,
    const std::string& trackId
) {
    const auto binding = repository.findCurrentByTrackId(trackId);
    if (!binding.has_value() || binding->status != domain::BindingStatus::Bound) {
        return std::nullopt;
    }

    return binding->employeeId;
}

void countDecision(
    const domain::AccessDecision& decision,
    MetadataDecisionTriggerResult& result
) {
    switch (decision.type) {
        case domain::AccessDecisionType::Allowed:
            ++result.allowed;
            break;
        case domain::AccessDecisionType::PendingIdentity:
            ++result.pendingIdentity;
            break;
        case domain::AccessDecisionType::Violation:
        case domain::AccessDecisionType::UnknownIdentity:
            ++result.violations;
            break;
        case domain::AccessDecisionType::Ignored:
            ++result.ignored;
            break;
    }
}

void countAlarmAction(
    alarm::AlarmPersistenceAction action,
    MetadataDecisionTriggerResult& result
) {
    switch (action) {
        case alarm::AlarmPersistenceAction::Created:
            ++result.alarmsCreated;
            break;
        case alarm::AlarmPersistenceAction::Resolved:
            ++result.alarmsResolved;
            break;
        case alarm::AlarmPersistenceAction::None:
            break;
    }
}

}

MetadataDecisionTriggerService::MetadataDecisionTriggerService(
    const geometry::ZoneGeometryService& zoneGeometryService,
    const decision::DecisionContextLoader& decisionContextLoader,
    const decision::DecisionEngine& decisionEngine,
    alarm::AlarmPersistenceService& alarmPersistenceService,
    const repository::ITrackIdentityBindingRepository& trackIdentityBindingRepository,
    const repository::IAlarmRepository& alarmRepository
) : zoneGeometryService_{zoneGeometryService},
    decisionContextLoader_{decisionContextLoader},
    decisionEngine_{decisionEngine},
    alarmPersistenceService_{alarmPersistenceService},
    trackIdentityBindingRepository_{trackIdentityBindingRepository},
    alarmRepository_{alarmRepository} {
}

MetadataDecisionTriggerResult MetadataDecisionTriggerService::trigger(
    const MetadataDecisionTriggerRequest& request
) {
    MetadataDecisionTriggerResult result{};

    for (const auto& detection : request.ingestionResult.detections) {
        ++result.detectionsChecked;

        if (detection.objectClass != domain::ObjectClass::Person) {
            ++result.ignored;
            continue;
        }

        for (const auto& zone : request.candidateZones) {
            if (zone.cameraId != detection.cameraId) {
                continue;
            }

            const bool isInsideZone = zoneGeometryService_.containsBoundingBoxCenter(
                zone,
                detection.bbox
            );
            const auto activeAlarm = alarmRepository_.findActiveByTrackAndZone(
                detection.trackId,
                zone.zoneId
            );

            if (!isInsideZone && !activeAlarm.has_value()) {
                continue;
            }

            const decision::DecisionContextRequest contextRequest{
                detection,
                zone.zoneId,
                boundEmployeeId(trackIdentityBindingRepository_, detection.trackId),
                isInsideZone,
                activeAlarm.has_value(),
                request.isIdentityGracePeriodActive
            };

            const auto loadedContext = decisionContextLoader_.load(contextRequest);
            if (!loadedContext.has_value()) {
                continue;
            }

            const auto context = loadedContext->toDecisionContext();
            const auto decision = decisionEngine_.evaluate(context);
            const auto alarmAction = alarmPersistenceService_.persist(
                decision,
                context,
                detection.timestamp
            );

            ++result.decisionsEvaluated;
            countDecision(decision, result);
            countAlarmAction(alarmAction, result);
        }
    }

    return result;
}

}
