#include "securezone/api/ApiServer.h"

namespace securezone::api {

ApiServer::ApiServer(ApiSettings settings)
    : settings_{std::move(settings)},
      xprotectEventRoutes_{{}, settings_.xprotectApiKey} {
    registerRoutes();
}

ApiServer::ApiServer(ApiSettings settings, QrRoutes::CheckInHandler qrCheckInHandler)
    : settings_{std::move(settings)},
      qrRoutes_{std::move(qrCheckInHandler), {}},
      xprotectEventRoutes_{{}, settings_.xprotectApiKey} {
    registerRoutes();
}

ApiServer::ApiServer(ApiSettings settings, ApiRouteHandlers handlers)
    : settings_{std::move(settings)},
      authRoutes_{std::move(handlers.loginHandler)},
      cameraObjectRoutes_{std::move(handlers.cameraObjectObservationHandler), settings_.xprotectApiKey},
      qrRoutes_{std::move(handlers.qrCheckInHandler), std::move(handlers.authorizationHandler)},
      xprotectEventRoutes_{std::move(handlers.lineCrossingHandler), settings_.xprotectApiKey} {
    registerRoutes();
}

const ApiSettings& ApiServer::settings() const {
    return settings_;
}

HttpResponse ApiServer::handle(const HttpRequest& request) const {
    return router_.route(request);
}

void ApiServer::registerRoutes() {
    router_.get("/health", [this](const HttpRequest& request) {
        return healthRoutes_.handleHealth(request);
    });

    router_.post("/api/auth/login", [this](const HttpRequest& request) {
        return authRoutes_.handleLogin(request);
    });

    router_.post("/api/qr/check-in", [this](const HttpRequest& request) {
        return qrRoutes_.handleCheckIn(request);
    });

    router_.post("/api/xprotect/line-crossing", [this](const HttpRequest& request) {
        return xprotectEventRoutes_.handleLineCrossing(request);
    });

    router_.post("/api/xprotect/object-observations", [this](const HttpRequest& request) {
        return cameraObjectRoutes_.handleObservation(request);
    });
}

}
