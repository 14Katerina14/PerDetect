#include "securezone/api/routes/XProtectEventRoutes.h"

#include "securezone/api/ApiResponse.h"
#include "securezone/api/events/XProtectLineCrossingEvent.h"

#include <sstream>
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

int httpStatusForLineCrossingResult(const XProtectLineCrossingResult& result) {
    if (result.accepted && result.status == "processed") {
        return 200;
    }

    if (result.accepted) {
        return 202;
    }

    if (result.status == "invalid_request") {
        return 400;
    }

    if (result.status == "zone_not_found") {
        return 404;
    }

    if (result.status == "handler_error") {
        return 500;
    }

    return 400;
}

}

XProtectEventRoutes::XProtectEventRoutes(
    LineCrossingHandler lineCrossingHandler,
    std::string apiKey
) : lineCrossingHandler_{std::move(lineCrossingHandler)},
    apiKey_{std::move(apiKey)} {
}

HttpResponse XProtectEventRoutes::handleLineCrossing(const HttpRequest& request) const {
    if (!apiKey_.empty()) {
        const auto apiKey = request.headers.find("X-SecureZone-Api-Key");
        if (apiKey == request.headers.end() || apiKey->second != apiKey_) {
            return jsonResponse(
                401,
                R"({"accepted":false,"status":"unauthorized","message":"A valid XProtect API key is required."})"
            );
        }
    }

    const auto result = parseXProtectLineCrossingEvent(request.body);
    if (!result.event) {
        return jsonBadRequest(
            R"({"accepted":false,"status":"rejected","reason":")" + result.error + R"("})"
        );
    }

    if (!lineCrossingHandler_) {
        return jsonResponse(
            503,
            R"({"accepted":false,"status":"service_unavailable","message":"XProtect line crossing handler is not configured."})"
        );
    }

    const auto handlerResult = lineCrossingHandler_(*result.event);
    std::ostringstream body;
    body << R"({"accepted":)" << (handlerResult.accepted ? "true" : "false")
         << R"(,"status":")"
         << jsonEscape(handlerResult.status)
         << R"(","decision":")"
         << jsonEscape(handlerResult.decision)
         << R"(","zoneId":")"
         << jsonEscape(handlerResult.zoneId)
         << R"(","sessionId":")"
         << jsonEscape(handlerResult.sessionId)
         << R"(","employeeId":")"
         << jsonEscape(handlerResult.employeeId)
         << R"(","message":")"
         << jsonEscape(handlerResult.message)
         << R"(","eventId":")"
         << jsonEscape(result.event->eventId)
         << R"(","duplicate":)"
         << (handlerResult.duplicate ? "true" : "false")
         << R"(,"eventType":"xprotect_line_crossing","eventName":")"
         << jsonEscape(result.event->eventName)
         << R"(","sourceName":")"
         << jsonEscape(result.event->sourceName)
         << R"(","receivedAt":")"
         << jsonEscape(result.event->receivedAt)
         << R"("})";

    return jsonResponse(httpStatusForLineCrossingResult(handlerResult), body.str());
}

}
