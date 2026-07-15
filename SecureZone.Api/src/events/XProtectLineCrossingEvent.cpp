#include "securezone/api/events/XProtectLineCrossingEvent.h"

#include <regex>

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

}

bool isWiseAiLineCrossingEvent(const std::string& eventName) {
    return eventName.find("OpenSDK.WiseAI.LineCrossing") != std::string::npos;
}

XProtectLineCrossingParseResult parseXProtectLineCrossingEvent(const std::string& body) {
    const auto eventName = readJsonStringField(body, "eventName");
    if (!eventName || eventName->empty()) {
        return {std::nullopt, "missing_event_name"};
    }

    const auto sourceName = readJsonStringField(body, "sourceName");
    if (!sourceName || sourceName->empty()) {
        return {std::nullopt, "missing_source_name"};
    }

    if (!isWiseAiLineCrossingEvent(*eventName)) {
        return {std::nullopt, "unsupported_xprotect_event"};
    }

    XProtectLineCrossingEvent event{};
    event.eventId = readJsonStringField(body, "eventId").value_or("");
    event.eventName = *eventName;
    event.sourceName = *sourceName;
    event.receivedAt = readJsonStringField(body, "receivedAt").value_or("");
    return {event, {}};
}

}
