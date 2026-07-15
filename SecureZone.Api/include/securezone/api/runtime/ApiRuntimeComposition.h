#pragma once

#include "securezone/api/ApiApplication.h"
#include "securezone/domain/AppUser.h"
#include "securezone/domain/Employee.h"
#include "securezone/domain/PresenceSession.h"
#include "securezone/domain/Zone.h"

#include <chrono>
#include <memory>
#include <string>
#include <vector>

namespace securezone::api {

struct XProtectZoneEventMapping {
    std::string eventName;
    std::string sourceName;
    std::string zoneId;
};

struct ApiRuntimeConfig {
    ApiSettings apiSettings{};
    ApiApplicationInfo appInfo{};
    std::vector<domain::Employee> employees;
    std::vector<domain::AppUser> appUsers;
    std::vector<domain::Zone> zones;
    std::vector<domain::PresenceSession> presenceSessions;
    std::vector<XProtectZoneEventMapping> xprotectZoneMappings;
    std::chrono::minutes qrPresenceDuration{2};
};

class ApiRuntimeComposition {
public:
    explicit ApiRuntimeComposition(ApiRuntimeConfig config = {});

    ApiApplication createApplication() const;
    ApiRouteHandlers createRouteHandlers() const;
    QrRoutes::CheckInHandler createQrCheckInHandler() const;
    XProtectEventRoutes::LineCrossingHandler createXProtectLineCrossingHandler() const;

private:
    struct State;

    std::shared_ptr<State> state_;
};

ApiApplication createComposedApiApplication(ApiRuntimeConfig config = {});

}
