#pragma once

#include <chrono>
#include <string>

namespace securezone::api {

struct ApiSettings {
    std::string host{"0.0.0.0"};
    int port{8080};
    std::string mongoConnectionString;
    std::string mongoDatabaseName{"securezone"};
    std::string xprotectApiKey;
    std::chrono::minutes qrPresenceDuration{2};
    std::string jwtSecret;
    std::chrono::minutes jwtTtl{60};
    std::chrono::seconds unidentifiedIdentityGracePeriod{120};
};

void validateProductionApiSettings(const ApiSettings& settings);
ApiSettings loadApiSettingsFromEnvironment();

}
