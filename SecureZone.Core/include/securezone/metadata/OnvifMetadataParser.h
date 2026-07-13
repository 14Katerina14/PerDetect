#pragma once

#include "securezone/metadata/IMetadataParser.h"

namespace securezone::metadata {

class OnvifMetadataParser final : public IMetadataParser {
public:
    ParsedMetadataFrame parse(
        const std::string& cameraId,
        const std::string& rawMetadata
    ) const override;
};

}
