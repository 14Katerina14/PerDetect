#include "securezone/api/routes/XProtectEventRoutes.h"

#include "securezone/api/ApiResponse.h"

namespace securezone::api {

HttpResponse XProtectEventRoutes::handleLineCrossing(const HttpRequest& request) const {
    (void)request;
    return jsonNotImplemented(
        R"({"accepted":false,"status":"not_implemented","message":"XProtect LineCrossing endpoint is reserved for event workflow implementation."})"
    );
}

}
