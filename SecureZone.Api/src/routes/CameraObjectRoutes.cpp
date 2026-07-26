#include "securezone/api/routes/CameraObjectRoutes.h"

#include "securezone/api/ApiResponse.h"
#include "securezone/api/handlers/XProtectLineCrossingHandler.h"

#include <optional>
#include <regex>
#include <utility>
#include <sstream>

namespace securezone::api {
namespace {

std::optional<std::string> field(const std::string& body, const std::string& name) {
    const std::regex pattern{"\"" + name + "\"\\s*:\\s*\"([^\"]*)\""};
    std::smatch match;
    if (!std::regex_search(body, match, pattern)) return std::nullopt;
    return match[1].str();
}

std::string jsonEscape(const std::string& value) {
    std::string escaped;
    for (const auto character : value) {
        if (character == '"') escaped += "\\\"";
        else if (character == '\\') escaped += "\\\\";
        else escaped += character;
    }
    return escaped;
}

}

CameraObjectRoutes::CameraObjectRoutes(ObservationHandler handler, std::string apiKey)
    : handler_{std::move(handler)}, apiKey_{std::move(apiKey)} {
}

HttpResponse CameraObjectRoutes::handleObservation(const HttpRequest& request) const {
    if (!apiKey_.empty()) {
        const auto key = request.headers.find("X-SecureZone-Api-Key");
        if (key == request.headers.end() || key->second != apiKey_) {
            return jsonResponse(401, R"({"accepted":false,"status":"unauthorized"})");
        }
    }

    const auto cameraId = field(request.body, "cameraId").value_or("");
    const auto objectId = field(request.body, "objectId").value_or("");
    const auto objectType = field(request.body, "objectType").value_or("");
    const auto objectStatus = field(request.body, "status").value_or("active");
    const auto observedAtText = field(request.body, "observedAt").value_or("");
    const auto observedAt = XProtectLineCrossingHandler::parseReceivedAt(observedAtText);
    if (cameraId.empty() || objectId.empty() || objectType.empty() || !observedAt.has_value()) {
        return jsonResponse(400, R"({"accepted":false,"status":"invalid_request"})");
    }
    if (!handler_) {
        return jsonResponse(503, R"({"accepted":false,"status":"service_unavailable"})");
    }

    identity::CameraObjectObservationStatus status{};
    if (objectStatus == "active") {
        status = identity::CameraObjectObservationStatus::Active;
    } else if (objectStatus == "lost") {
        status = identity::CameraObjectObservationStatus::Lost;
    } else {
        return jsonResponse(400, R"({"accepted":false,"status":"invalid_object_status"})");
    }

    const auto result = handler_({cameraId, objectId, objectType, *observedAt, status});
    std::ostringstream body;
    body << R"({"accepted":)" << (result.accepted ? "true" : "false")
         << R"(,"status":")" << jsonEscape(result.status)
         << R"(","decision":")" << jsonEscape(result.decision)
         << R"(","zoneId":")" << jsonEscape(result.zoneId)
         << R"(","message":")" << jsonEscape(result.message)
         << R"(","eventId":")" << jsonEscape(result.eventId)
         << R"(","duplicate":)" << (result.duplicate ? "true" : "false") << "}";
    return jsonResponse(result.accepted ? 200 : 400, body.str());
}

}
