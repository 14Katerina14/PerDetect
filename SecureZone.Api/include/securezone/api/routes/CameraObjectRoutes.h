#pragma once

#include "securezone/api/http/HttpRequest.h"
#include "securezone/api/http/HttpResponse.h"
#include "securezone/identity/CameraIdentityService.h"

#include <functional>
#include <string>

namespace securezone::api {

class CameraObjectRoutes {
public:
    using ObservationHandler = std::function<bool(const identity::CameraObjectObservation&)>;

    CameraObjectRoutes() = default;
    CameraObjectRoutes(ObservationHandler handler, std::string apiKey);

    HttpResponse handleObservation(const HttpRequest& request) const;

private:
    ObservationHandler handler_;
    std::string apiKey_;
};

}
