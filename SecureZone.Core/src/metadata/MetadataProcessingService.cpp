#include "securezone/metadata/MetadataProcessingService.h"

namespace securezone::metadata {

MetadataProcessingService::MetadataProcessingService(
    const MetadataIngestionService& ingestionService,
    MetadataPersistenceService& persistenceService
) : ingestionService_{ingestionService},
    persistenceService_{persistenceService} {
}

MetadataProcessingResult MetadataProcessingService::process(
    const std::string& cameraId,
    const std::string& rawMetadata
) {
    const auto ingestionResult = ingestionService_.ingest(cameraId, rawMetadata);
    const auto persistenceResult = persistenceService_.persist(ingestionResult);

    return MetadataProcessingResult{
        ingestionResult.detections.size(),
        persistenceResult.tracksUpserted,
        persistenceResult.eventsCreated
    };
}

}
