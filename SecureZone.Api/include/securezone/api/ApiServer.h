#pragma once

#include "securezone/api/ApiSettings.h"
#include "securezone/api/http/HttpRequest.h"
#include "securezone/api/http/HttpResponse.h"
#include "securezone/api/http/Router.h"
#include "securezone/api/routes/HealthRoutes.h"
#include "securezone/api/routes/QrRoutes.h"
#include "securezone/api/routes/XProtectEventRoutes.h"

namespace securezone::api {

class ApiServer {
public:
    explicit ApiServer(ApiSettings settings = {});
    ApiServer(ApiSettings settings, QrRoutes::CheckInHandler qrCheckInHandler);

    const ApiSettings& settings() const;
    HttpResponse handle(const HttpRequest& request) const;

private:
    void registerRoutes();

    ApiSettings settings_;
    Router router_;
    HealthRoutes healthRoutes_;
    QrRoutes qrRoutes_;
    XProtectEventRoutes xprotectEventRoutes_;
};

}
