#pragma once

#include <chrono>
#include <string>
#include "securezone/domain/BoundingBox.h"

namespace securezone::domain {
enum class ObjectClass { Person, Vehicle, Unknown };
struct Detection {
    std::string trackId, cameraId;
    ObjectClass objectClass{ObjectClass::Unknown};
    BoundingBox bbox;
    double confidence{};
    std::chrono::system_clock::time_point timestamp{};
};
}
