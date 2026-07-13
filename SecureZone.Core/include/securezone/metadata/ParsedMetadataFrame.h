#pragma once

#include <chrono>
#include <string>
#include <vector>

#include "securezone/domain/Detection.h"
#include "securezone/domain/MetadataEvent.h"

namespace securezone::metadata {

struct ParsedMetadataFrame {
    std::string cameraId;
    std::string source;
    std::chrono::system_clock::time_point timestamp{};
    std::vector<domain::Detection> detections;
    std::vector<domain::MetadataEvent> events;
};

}
