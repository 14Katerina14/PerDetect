#include "securezone/metadata/MetadataApplicationService.h"

namespace securezone::metadata {

MetadataApplicationService::MetadataApplicationService(
    MetadataProcessingService& processingService,
    MetadataDecisionTriggerService& decisionTriggerService
) : processingService_{processingService},
    decisionTriggerService_{decisionTriggerService} {
}

MetadataApplicationResult MetadataApplicationService::handle(
    const MetadataApplicationRequest& request
) {
    const auto processingOutcome = processingService_.processDetailed(
        request.cameraId,
        request.rawMetadata
    );

    const auto decisionResult = decisionTriggerService_.trigger(
        MetadataDecisionTriggerRequest{
            processingOutcome.ingestionResult,
            request.candidateZones,
            request.isIdentityGracePeriodActive
        }
    );

    return MetadataApplicationResult{
        processingOutcome.result,
        decisionResult
    };
}

}
