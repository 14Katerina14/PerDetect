#include "securezone/api/routes/CameraObjectRoutes.h"

#include "securezone/api/ApiResponse.h"
#include "securezone/api/handlers/XProtectLineCrossingHandler.h"

#include <optional>
#include <regex>
#include <utility>

namespace securezone::api {
namespace {

std::optional<std::string> field(const std::string& body, const std::string& name) {
    const std::regex pattern{"\"" + name + "\"\\s*:\\s*\"([^\"]*)\""};
    std::smatch match;
    if (!std::regex_search(body, match, pattern)) return std::nullopt;
    return match[1].str();
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
    const auto observedAtText = field(request.body, "observedAt").value_or("");
    const auto observedAt = XProtectLineCrossingHandler::parseReceivedAt(observedAtText);
    if (cameraId.empty() || objectId.empty() || objectType.empty() || !observedAt.has_value()) {
        return jsonResponse(400, R"({"accepted":false,"status":"invalid_request"})");
    }
    if (!handler_) {
        return jsonResponse(503, R"({"accepted":false,"status":"service_unavailable"})");
    }

    const bool accepted = handler_({cameraId, objectId, objectType, *observedAt});
    return accepted
        ? jsonResponse(202, R"({"accepted":true,"status":"observed"})")
        : jsonResponse(400, R"({"accepted":false,"status":"rejected"})");
}

}
