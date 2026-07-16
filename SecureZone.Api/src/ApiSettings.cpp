#include "securezone/api/ApiSettings.h"

#include <cstdlib>
#include <stdexcept>
#include <string>

namespace securezone::api {

namespace {

std::string envString(const char* name, std::string fallback = {}) {
    if (const auto* value = std::getenv(name); value != nullptr && value[0] != '\0') {
        return value;
    }

    return fallback;
}

int envInt(const char* name, int fallback) {
    const auto value = envString(name);
    if (value.empty()) {
        return fallback;
    }

    try {
        return std::stoi(value);
    } catch (...) {
        return fallback;
    }
}

}

ApiSettings loadApiSettingsFromEnvironment() {
    ApiSettings settings{};
    settings.host = envString("SECUREZONE_API_HOST", settings.host);
    settings.port = envInt("SECUREZONE_API_PORT", settings.port);
    settings.mongoConnectionString = envString("SECUREZONE_MONGO_URI");
    settings.mongoDatabaseName = envString("SECUREZONE_MONGO_DATABASE", settings.mongoDatabaseName);
    settings.xprotectApiKey = envString("SECUREZONE_XPROTECT_API_KEY");
    settings.qrPresenceDuration = std::chrono::minutes{
        envInt("SECUREZONE_QR_PRESENCE_MINUTES", static_cast<int>(settings.qrPresenceDuration.count()))
    };
    settings.jwtSecret = envString("SECUREZONE_JWT_SECRET");
    settings.jwtTtl = std::chrono::minutes{
        envInt("SECUREZONE_JWT_TTL_MINUTES", static_cast<int>(settings.jwtTtl.count()))
    };
    validateProductionApiSettings(settings);
    return settings;
}

void validateProductionApiSettings(const ApiSettings& settings) {
    if (settings.jwtSecret.size() < 32) {
        throw std::runtime_error(
            "SECUREZONE_JWT_SECRET is required and must contain at least 32 bytes."
        );
    }
    if (settings.jwtTtl.count() <= 0) {
        throw std::runtime_error("SECUREZONE_JWT_TTL_MINUTES must be a positive integer.");
    }
}

}
