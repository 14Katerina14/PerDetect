#pragma once

#include <string>
#include <vector>

#include "securezone/domain/Zone.h"
#include "securezone/metadata/MetadataApplicationResult.h"
#include "securezone/metadata/MetadataDecisionTriggerService.h"
#include "securezone/metadata/MetadataProcessingService.h"

namespace securezone::metadata {

struct MetadataApplicationRequest {
    std::string cameraId;
    std::string rawMetadata;
    std::vector<domain::Zone> candidateZones;
    bool isIdentityGracePeriodActive{};
};

class MetadataApplicationService {
public:
    MetadataApplicationService(
        MetadataProcessingService& processingService,
        MetadataDecisionTriggerService& decisionTriggerService
    );

    MetadataApplicationResult handle(const MetadataApplicationRequest& request);

private:
    MetadataProcessingService& processingService_;
    MetadataDecisionTriggerService& decisionTriggerService_;
};

}
