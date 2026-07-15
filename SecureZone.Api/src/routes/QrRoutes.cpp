#include "securezone/api/routes/QrRoutes.h"

#include "securezone/api/ApiResponse.h"

#include <optional>
#include <regex>
#include <sstream>
#include <string>
#include <utility>

namespace securezone::api {
namespace {

std::optional<std::string> readJsonStringField(const std::string& body, const std::string& fieldName) {
    const std::regex fieldPattern{"\"" + fieldName + "\"\\s*:\\s*\"([^\"]*)\""};
    std::smatch match;
    if (!std::regex_search(body, match, fieldPattern)) {
        return std::nullopt;
    }

    return match[1].str();
}

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

QrRoutes::QrRoutes(CheckInHandler checkInHandler)
    : checkInHandler_{std::move(checkInHandler)} {
}

HttpResponse QrRoutes::handleCheckIn(const HttpRequest& request) const {
    if (!checkInHandler_) {
        return jsonResponse(
            503,
            R"({"accepted":false,"status":"service_unavailable","message":"QR check-in service is not configured."})"
        );
    }

    qr::QrCheckInCommand command{};
    command.employeeId = readJsonStringField(request.body, "employeeId").value_or("");
    command.zoneId = readJsonStringField(request.body, "zoneId").value_or("");
    command.scannedByUserId = readJsonStringField(request.body, "scannedByUserId").value_or("");
    command.cameraId = readJsonStringField(request.body, "cameraId").value_or("");

    return qrResultResponse(checkInHandler_(command));
}

}
