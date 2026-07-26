#pragma once

#include "securezone/api/ApiSettings.h"
#include "securezone/api/http/HttpRequest.h"
#include "securezone/api/http/HttpResponse.h"
#include "securezone/api/http/Router.h"
#include "securezone/api/routes/HealthRoutes.h"
#include "securezone/api/routes/AlarmRoutes.h"
#include "securezone/api/routes/AuthRoutes.h"
#include "securezone/api/routes/CameraObjectRoutes.h"
#include "securezone/api/routes/QrRoutes.h"
#include "securezone/api/routes/XProtectEventRoutes.h"

namespace securezone::api {

struct ApiRouteHandlers {
    QrRoutes::CheckInHandler qrCheckInHandler{};
    XProtectEventRoutes::LineCrossingHandler lineCrossingHandler{};
    CameraObjectRoutes::ObservationHandler cameraObjectObservationHandler{};
    AuthRoutes::LoginHandler loginHandler{};
    EndpointAuthorizer::Handler authorizationHandler{};
    AlarmRoutes::ListHandler activeAlarmsHandler{};
    AlarmRoutes::ListHandler recentAlarmsHandler{};
};

class ApiServer {
public:
    explicit ApiServer(ApiSettings settings = {});
    ApiServer(ApiSettings settings, QrRoutes::CheckInHandler qrCheckInHandler);
    ApiServer(ApiSettings settings, ApiRouteHandlers handlers);

    const ApiSettings& settings() const;
    HttpResponse handle(const HttpRequest& request) const;

private:
    void registerRoutes();

    ApiSettings settings_;
    Router router_;
    HealthRoutes healthRoutes_;
    AlarmRoutes alarmRoutes_;
    AuthRoutes authRoutes_;
    CameraObjectRoutes cameraObjectRoutes_;
    QrRoutes qrRoutes_;
    XProtectEventRoutes xprotectEventRoutes_;
};

}
