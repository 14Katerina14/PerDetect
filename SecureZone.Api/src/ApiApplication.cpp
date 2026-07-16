#include "securezone/api/ApiApplication.h"

#include "securezone/api/runtime/ApiRuntimeComposition.h"
#include "securezone/api/ApiResponse.h"
#ifdef SECUREZONE_WITH_MONGODB_INFRA
#include "securezone/api/runtime/MongoApiRuntimeComposition.h"
#endif

#include <sstream>
#include <cstdlib>
#include <utility>

#include <nlohmann/json.hpp>

namespace securezone::api {

ApiApplication::ApiApplication(ApiSettings settings, ApiApplicationInfo info)
    : settings_{std::move(settings)},
      info_{std::move(info)},
      server_{settings_} {
}

ApiApplication::ApiApplication(
    ApiSettings settings,
    ApiApplicationInfo info,
    QrRoutes::CheckInHandler qrCheckInHandler
) : settings_{std::move(settings)},
    info_{std::move(info)},
    server_{settings_, std::move(qrCheckInHandler)} {
}

ApiApplication::ApiApplication(
    ApiSettings settings,
    ApiApplicationInfo info,
    ApiRouteHandlers handlers
) : settings_{std::move(settings)},
    info_{std::move(info)},
    server_{settings_, std::move(handlers)} {
}

const ApiSettings& ApiApplication::settings() const {
    return settings_;
}

const ApiApplicationInfo& ApiApplication::info() const {
    return info_;
}

HttpResponse ApiApplication::handle(const HttpRequest& request) const {
    if (request.path == "/version") {
        if (request.method != "GET") {
            return jsonMethodNotAllowed(R"({"error":"method_not_allowed"})");
        }

        return jsonOk(nlohmann::json{
            {"service", info_.serviceName},
            {"version", info_.version},
            {"buildId", info_.buildId}
        }.dump());
    }

    return server_.handle(request);
}

std::string ApiApplication::startupSummary() const {
    std::ostringstream output;
    output << info_.serviceName << ' ' << info_.version
           << " (build " << info_.buildId << ')'
           << " configured for " << settings_.host << ':' << settings_.port
           << " using database " << settings_.mongoDatabaseName;
    return output.str();
}

ApiApplication createApiApplicationFromEnvironment() {
#ifdef SECUREZONE_WITH_MONGODB_INFRA
    return createMongoApiApplicationFromEnvironment();
#else
    ApiRuntimeConfig config{};
    config.apiSettings = loadApiSettingsFromEnvironment();
    config.appInfo = loadApiApplicationInfoFromEnvironment();
    return createComposedApiApplication(std::move(config));
#endif
}

ApiApplicationInfo loadApiApplicationInfoFromEnvironment() {
    ApiApplicationInfo info{};
    if (const auto* buildId = std::getenv("SECUREZONE_BUILD_ID");
        buildId != nullptr && buildId[0] != '\0') {
        info.buildId = buildId;
    }
    return info;
}

}
