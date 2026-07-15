#include "securezone/api/routes/QrRoutes.h"

#include "securezone/api/ApiResponse.h"

namespace securezone::api {

HttpResponse QrRoutes::handleCheckIn(const HttpRequest& request) const {
    (void)request;
    return jsonNotImplemented(
        R"({"accepted":false,"status":"not_implemented","message":"QR check-in endpoint is reserved for QrRoutes implementation."})"
    );
}

}
