#pragma once

#include <string>

#include "securezone/metadata/MetadataIngestionService.h"
#include "securezone/metadata/MetadataPersistenceService.h"
#include "securezone/metadata/MetadataProcessingOutcome.h"
#include "securezone/metadata/MetadataProcessingResult.h"

namespace securezone::metadata {

class MetadataProcessingService {
public:
    MetadataProcessingService(
        const MetadataIngestionService& ingestionService,
        MetadataPersistenceService& persistenceService
    );

    MetadataProcessingResult process(
        const std::string& cameraId,
        const std::string& rawMetadata
    );

    MetadataProcessingOutcome processDetailed(
        const std::string& cameraId,
        const std::string& rawMetadata
    );

private:
    const MetadataIngestionService& ingestionService_;
    MetadataPersistenceService& persistenceService_;
};

}
