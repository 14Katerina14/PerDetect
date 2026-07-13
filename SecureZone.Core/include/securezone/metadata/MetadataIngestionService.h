#pragma once

#include <string>

#include "securezone/metadata/IMetadataParser.h"
#include "securezone/metadata/MetadataIngestionResult.h"

namespace securezone::metadata {

class MetadataIngestionService {
public:
    explicit MetadataIngestionService(const IMetadataParser& parser);

    MetadataIngestionResult ingest(
        const std::string& cameraId,
        const std::string& rawMetadata
    ) const;

private:
    const IMetadataParser& parser_;
};

}
