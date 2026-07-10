#pragma once

#include <string>
#include <vector>
#include "securezone/domain/Point.h"

namespace securezone::domain {
enum class ZoneType { Safe, Restricted, Dangerous };
enum class ZoneStatus { Active, Inactive };
struct Zone {
    std::string zoneId, name, cameraId, relatedMachineId;
    ZoneType type{ZoneType::Restricted};
    ZoneStatus status{ZoneStatus::Active};
    std::vector<Point> polygon;
};
}
