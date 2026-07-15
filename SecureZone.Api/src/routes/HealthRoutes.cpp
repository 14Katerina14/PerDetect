#include "securezone/api/routes/HealthRoutes.h"

#include "securezone/api/ApiResponse.h"

namespace securezone::api {

HttpResponse HealthRoutes::handleHealth(const HttpRequest& request) const {
    (void)request;
    return jsonOk(R"({"status":"ok","service":"securezone-api"})");
}

}
