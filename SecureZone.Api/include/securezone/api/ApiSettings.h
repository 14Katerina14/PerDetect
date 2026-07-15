#pragma once

#include <chrono>
#include <string>

namespace securezone::api {

struct ApiSettings {
    std::string host{"0.0.0.0"};
    int port{8080};
    std::string mongoConnectionString;
    std::string mongoDatabaseName{"securezone"};
    std::chrono::minutes qrPresenceDuration{2};
};

ApiSettings loadApiSettingsFromEnvironment();

}
