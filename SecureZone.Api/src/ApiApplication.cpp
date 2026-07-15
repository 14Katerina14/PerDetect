#include "securezone/api/ApiApplication.h"

#include <sstream>
#include <utility>

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

const ApiSettings& ApiApplication::settings() const {
    return settings_;
}

const ApiApplicationInfo& ApiApplication::info() const {
    return info_;
}

HttpResponse ApiApplication::handle(const HttpRequest& request) const {
    return server_.handle(request);
}

std::string ApiApplication::startupSummary() const {
    std::ostringstream output;
    output << info_.serviceName << ' ' << info_.version
           << " configured for " << settings_.host << ':' << settings_.port
           << " using database " << settings_.mongoDatabaseName;
    return output.str();
}

ApiApplication createApiApplicationFromEnvironment() {
    return ApiApplication{loadApiSettingsFromEnvironment(), {}};
}

}
