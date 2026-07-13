#pragma once

#include "securezone/metadata/MetadataIngestionResult.h"
#include "securezone/metadata/MetadataProcessingResult.h"

namespace securezone::metadata {

struct MetadataProcessingOutcome {
    MetadataProcessingResult result;
    MetadataIngestionResult ingestionResult;
};

}
