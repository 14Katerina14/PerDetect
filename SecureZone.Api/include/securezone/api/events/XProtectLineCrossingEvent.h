#pragma once

#include <optional>
#include <string>

namespace securezone::api {

struct XProtectLineCrossingEvent {
    std::string eventId;
    std::string eventName;
    std::string sourceName;
    std::string receivedAt;
};

struct XProtectLineCrossingParseResult {
    std::optional<XProtectLineCrossingEvent> event;
    std::string error;
};

struct XProtectLineCrossingResult {
    bool accepted{};
    std::string status;
    std::string decision;
    std::string zoneId;
    std::string sessionId;
    std::string employeeId;
    std::string message;
    bool duplicate{};
};

XProtectLineCrossingParseResult parseXProtectLineCrossingEvent(const std::string& body);
bool isWiseAiLineCrossingEvent(const std::string& eventName);

}
