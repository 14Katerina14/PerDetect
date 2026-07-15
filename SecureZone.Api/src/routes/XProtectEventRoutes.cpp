#include "securezone/api/routes/XProtectEventRoutes.h"

#include "securezone/api/ApiResponse.h"
#include "securezone/api/events/XProtectLineCrossingEvent.h"

#include <sstream>

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

}

HttpResponse XProtectEventRoutes::handleLineCrossing(const HttpRequest& request) const {
    const auto result = parseXProtectLineCrossingEvent(request.body);
    if (!result.event) {
        return jsonBadRequest(
            R"({"accepted":false,"status":"rejected","reason":")" + result.error + R"("})"
        );
    }

    std::ostringstream body;
    body << R"({"accepted":true,"status":"accepted","eventType":"xprotect_line_crossing","eventName":")"
         << jsonEscape(result.event->eventName)
         << R"(","sourceName":")"
         << jsonEscape(result.event->sourceName)
         << R"(","receivedAt":")"
         << jsonEscape(result.event->receivedAt)
         << R"("})";

    return jsonAccepted(body.str());
}

}
