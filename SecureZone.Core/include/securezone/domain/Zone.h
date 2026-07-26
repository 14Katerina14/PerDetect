#pragma once

#include <string>

namespace securezone::domain {
enum class ZoneType { Safe, Restricted, Dangerous };
enum class ZoneStatus { Active, Inactive };
struct Zone {
    std::string zoneId, name, cameraId;
    ZoneType type{ZoneType::Restricted};
    ZoneStatus status{ZoneStatus::Active};
    std::string relatedMachineId;
    std::string xprotectEventName;
};
}
