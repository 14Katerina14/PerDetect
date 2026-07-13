#pragma once

#include <chrono>
#include <string>

#include "securezone/domain/BoundingBox.h"

namespace securezone::domain {

struct MetadataEvent {
    std::string eventId;
    std::string cameraId;
    std::string trackId;
    std::chrono::system_clock::time_point timestamp{};
    std::string objectClass;
    BoundingBox bbox;
    std::string zoneId;
    std::string eventType;
};

}
