#pragma once

#include <optional>
#include <string>

namespace securezone::api {

struct XProtectLineCrossingEvent {
    std::string eventName;
    std::string sourceName;
    std::string receivedAt;
};

struct XProtectLineCrossingParseResult {
    std::optional<XProtectLineCrossingEvent> event;
    std::string error;
};

XProtectLineCrossingParseResult parseXProtectLineCrossingEvent(const std::string& body);
bool isWiseAiLineCrossingEvent(const std::string& eventName);

}
