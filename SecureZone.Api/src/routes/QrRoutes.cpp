#include "securezone/api/routes/QrRoutes.h"

#include "securezone/api/ApiResponse.h"

#include <nlohmann/json.hpp>

#include <sstream>
#include <string>
#include <utility>

namespace securezone::api {
namespace {

std::string jsonEscape(const std::string& value) {
    std::string escaped;
    escaped.reserve(value.size());
    for (const auto character : value) {
        if (character == '"') {
            escaped += "\\\"";
        } else if (character == '\\') {
            escaped += "\\\\";
        } else {
            escaped += character;
        }
    }
    return escaped;
}

int httpStatusForQrResult(const qr::QrCheckInResult& result) {
    if (result.accepted && result.status == "started") {
        return 201;
    }

    if (result.accepted) {
        return 200;
    }

    if (result.status == "invalid_request") {
        return 400;
    }

    if (result.status == "no_recent_human") {
        return 409;
    }

    if (result.status == "employee_not_found"
        || result.status == "zone_not_found"
        || result.status == "scanner_not_found") {
        return 404;
    }

    if (result.status == "employee_inactive"
        || result.status == "zone_inactive"
        || result.status == "scanner_not_allowed") {
        return 403;
    }

    return 400;
}

HttpResponse qrResultResponse(const qr::QrCheckInResult& result) {
    std::ostringstream body;
    body << R"({"accepted":)" << (result.accepted ? "true" : "false")
         << R"(,"status":")" << jsonEscape(result.status)
         << R"(","sessionId":")" << jsonEscape(result.sessionId)
         << R"(","message":")" << jsonEscape(result.message)
         << R"(","objectId":")" << jsonEscape(result.objectId)
         << R"(","bindingId":")" << jsonEscape(result.bindingId)
         << R"("})";

    return jsonResponse(httpStatusForQrResult(result), body.str());
}

}

QrRoutes::QrRoutes(
    CheckInHandler checkInHandler,
    EndpointAuthorizer::Handler authorizeHandler
) : checkInHandler_{std::move(checkInHandler)},
    authorizeHandler_{std::move(authorizeHandler)} {
}

HttpResponse QrRoutes::handleCheckIn(const HttpRequest& request) const {
    if (!authorizeHandler_) {
        return jsonResponse(503, R"({"error":"authorization_unavailable"})");
    }

    const auto authorization = authorizeHandler_(request, auth::AuthorizationPolicy::Scanner);
    if (authorization.status == EndpointAuthorizationStatus::Unauthorized) {
        return jsonResponse(401, R"({"error":"unauthorized"})");
    }
    if (authorization.status == EndpointAuthorizationStatus::Forbidden) {
        return jsonResponse(403, R"({"error":"forbidden"})");
    }

    if (!checkInHandler_) {
        return jsonResponse(
            503,
            R"({"accepted":false,"status":"service_unavailable","message":"QR check-in service is not configured."})"
        );
    }

    const auto body = nlohmann::json::parse(request.body, nullptr, false);
    if (body.is_discarded() || !body.is_object()) {
        return jsonResponse(400, R"({"accepted":false,"status":"invalid_request"})");
    }

    const auto stringField = [&body](const char* name) {
        return body.contains(name) && body[name].is_string()
            ? body[name].get<std::string>()
            : std::string{};
    };

    qr::QrCheckInCommand command{};
    command.employeeId = stringField("employeeId");
    command.zoneId = stringField("zoneId");
    command.cameraId = stringField("cameraId");
    command.scannedByUserId = authorization.principal->userId;

    return qrResultResponse(checkInHandler_(command));
}

}
