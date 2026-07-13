#pragma once

#include <string>

#include "securezone/metadata/ParsedMetadataFrame.h"

namespace securezone::metadata {

class IMetadataParser {
public:
    virtual ~IMetadataParser() = default;

    virtual ParsedMetadataFrame parse(
        const std::string& cameraId,
        const std::string& rawMetadata
    ) const = 0;
};

}
