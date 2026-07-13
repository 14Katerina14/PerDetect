#include "securezone/metadata/MetadataPersistenceService.h"

namespace securezone::metadata {

namespace {

domain::CameraTrack mergeWithExistingTrack(
    const domain::CameraTrack& incomingTrack,
    const domain::CameraTrack& existingTrack
) {
    auto mergedTrack = incomingTrack;
    mergedTrack.firstSeenAt = existingTrack.firstSeenAt;
    mergedTrack.currentZoneId = existingTrack.currentZoneId;
    return mergedTrack;
}

}

MetadataPersistenceService::MetadataPersistenceService(
    repository::ICameraTrackRepository& cameraTrackRepository,
    repository::IMetadataEventRepository& metadataEventRepository
) : cameraTrackRepository_{cameraTrackRepository},
    metadataEventRepository_{metadataEventRepository} {
}

MetadataPersistenceResult MetadataPersistenceService::persist(
    const MetadataIngestionResult& ingestionResult
) {
    MetadataPersistenceResult result{};

    for (const auto& cameraTrack : ingestionResult.cameraTracks) {
        auto trackToPersist = cameraTrack;
        const auto existingTrack = cameraTrackRepository_.findByTrackId(cameraTrack.trackId);

        if (existingTrack.has_value()) {
            trackToPersist = mergeWithExistingTrack(cameraTrack, *existingTrack);
        }

        cameraTrackRepository_.upsert(trackToPersist);
        ++result.tracksUpserted;
    }

    for (const auto& metadataEvent : ingestionResult.metadataEvents) {
        metadataEventRepository_.create(metadataEvent);
        ++result.eventsCreated;
    }

    return result;
}

}
