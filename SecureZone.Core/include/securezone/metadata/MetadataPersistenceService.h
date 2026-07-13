#pragma once

#include "securezone/metadata/MetadataIngestionResult.h"
#include "securezone/metadata/MetadataPersistenceResult.h"
#include "securezone/repository/ICameraTrackRepository.h"
#include "securezone/repository/IMetadataEventRepository.h"

namespace securezone::metadata {

class MetadataPersistenceService {
public:
    MetadataPersistenceService(
        repository::ICameraTrackRepository& cameraTrackRepository,
        repository::IMetadataEventRepository& metadataEventRepository
    );

    MetadataPersistenceResult persist(const MetadataIngestionResult& ingestionResult);

private:
    repository::ICameraTrackRepository& cameraTrackRepository_;
    repository::IMetadataEventRepository& metadataEventRepository_;
};

}
