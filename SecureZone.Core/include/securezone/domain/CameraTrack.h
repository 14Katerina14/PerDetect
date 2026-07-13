#pragma once

#include <chrono>
#include <string>

#include "securezone/domain/BoundingBox.h"

namespace securezone::domain {

struct CameraTrack {
    std::string trackId;
    std::string cameraId;
    std::chrono::system_clock::time_point firstSeenAt{};
    std::chrono::system_clock::time_point lastSeenAt{};
    std::string currentZoneId;
    std::string objectClass;
    BoundingBox bbox;
    std::string status;
};

}
