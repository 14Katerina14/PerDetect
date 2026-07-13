#pragma once

#include <vector>

#include "securezone/domain/CameraTrack.h"
#include "securezone/domain/Detection.h"
#include "securezone/domain/MetadataEvent.h"

namespace securezone::metadata {

struct MetadataIngestionResult {
    std::vector<domain::Detection> detections;
    std::vector<domain::CameraTrack> cameraTracks;
    std::vector<domain::MetadataEvent> metadataEvents;
};

}
