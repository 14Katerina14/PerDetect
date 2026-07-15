#include "securezone/api/ApiServer.h"

namespace securezone::api {

ApiServer::ApiServer(ApiSettings settings)
    : settings_{std::move(settings)} {
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

    router_.post("/api/qr/check-in", [this](const HttpRequest& request) {
        return qrRoutes_.handleCheckIn(request);
    });

    router_.post("/api/xprotect/line-crossing", [this](const HttpRequest& request) {
        return xprotectEventRoutes_.handleLineCrossing(request);
    });
}

}
