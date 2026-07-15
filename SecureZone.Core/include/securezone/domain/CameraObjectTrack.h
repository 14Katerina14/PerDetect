#pragma once

#include <chrono>
#include <string>

namespace securezone::domain {

enum class CameraObjectTrackStatus { Active, Lost };

struct CameraObjectTrack {
    std::string cameraId;
    std::string objectId;
    std::string objectType;
    std::chrono::system_clock::time_point firstSeenAt{};
    std::chrono::system_clock::time_point lastSeenAt{};
    CameraObjectTrackStatus status{CameraObjectTrackStatus::Active};

    bool isHuman() const {
        return objectType == "Human" || objectType == "Person";
    }
};

}
