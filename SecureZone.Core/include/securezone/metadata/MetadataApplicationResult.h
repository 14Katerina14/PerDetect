#pragma once

#include "securezone/metadata/MetadataDecisionTriggerResult.h"
#include "securezone/metadata/MetadataProcessingResult.h"

namespace securezone::metadata {

struct MetadataApplicationResult {
    MetadataProcessingResult processing;
    MetadataDecisionTriggerResult decisions;
};

}
