#pragma once

#include <string>
#include <vector>
namespace securezone::domain {
enum class UiPermission { CanViewLiveCamera, CanAcknowledgeAlarm, CanEditZones, CanScanQr };
struct RolePermission { std::string role; std::vector<UiPermission> permissions; };
}
