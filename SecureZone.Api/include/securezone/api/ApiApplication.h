#pragma once

#include "securezone/api/ApiServer.h"
#include "securezone/api/ApiSettings.h"
#include "securezone/api/http/HttpRequest.h"
#include "securezone/api/http/HttpResponse.h"

#include <string>

namespace securezone::api {

struct ApiApplicationInfo {
    std::string serviceName{"securezone-api"};
    std::string version{"0.1.0"};
};

class ApiApplication {
public:
    explicit ApiApplication(ApiSettings settings = {}, ApiApplicationInfo info = {});
    ApiApplication(
        ApiSettings settings,
        ApiApplicationInfo info,
        QrRoutes::CheckInHandler qrCheckInHandler
    );
    ApiApplication(
        ApiSettings settings,
        ApiApplicationInfo info,
        ApiRouteHandlers handlers
    );

    const ApiSettings& settings() const;
    const ApiApplicationInfo& info() const;

    HttpResponse handle(const HttpRequest& request) const;
    std::string startupSummary() const;

private:
    ApiSettings settings_;
    ApiApplicationInfo info_;
    ApiServer server_;
};

ApiApplication createApiApplicationFromEnvironment();

}
