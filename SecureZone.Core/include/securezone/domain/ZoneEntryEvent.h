#pragma once

#include <chrono>
#include <string>

namespace securezone::domain {

struct ZoneEntryEvent {
    std::string eventId;
    std::string trackId;
    std::string cameraId;
    std::string sourceName;
    std::chrono::system_clock::time_point timestamp{};
};

}
