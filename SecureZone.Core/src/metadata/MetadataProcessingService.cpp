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
    return processDetailed(cameraId, rawMetadata).result;
}

MetadataProcessingOutcome MetadataProcessingService::processDetailed(
    const std::string& cameraId,
    const std::string& rawMetadata
) {
    const auto ingestionResult = ingestionService_.ingest(cameraId, rawMetadata);
    const auto persistenceResult = persistenceService_.persist(ingestionResult);

    MetadataProcessingOutcome outcome{};
    outcome.result = MetadataProcessingResult{
        ingestionResult.detections.size(),
        persistenceResult.tracksUpserted,
        persistenceResult.eventsCreated
    };
    outcome.ingestionResult = ingestionResult;
    return outcome;
}

}
