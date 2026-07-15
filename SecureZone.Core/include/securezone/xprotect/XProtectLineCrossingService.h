#pragma once

#include "securezone/domain/PresenceSession.h"
#include "securezone/domain/Zone.h"
#include "securezone/domain/TrackIdentityBinding.h"

#include <chrono>
#include <functional>
#include <optional>
#include <string>

namespace securezone::xprotect {

struct XProtectLineCrossingCommand {
    std::string eventName;
    std::string sourceName;
    std::chrono::system_clock::time_point receivedAt{};
    std::string cameraId;
    std::string objectId;
    std::string action;
};

struct XProtectLineCrossingDecision {
    bool accepted{};
    std::string status;
    std::string decision;
    std::string zoneId;
    std::string sessionId;
    std::string employeeId;
    std::string message;
};

class XProtectLineCrossingService {
public:
    using Clock = std::chrono::system_clock;
    using ZoneResolver = std::function<std::optional<domain::Zone>(const XProtectLineCrossingCommand&)>;
    using ActivePresenceResolver = std::function<std::optional<domain::PresenceSession>(
        const domain::Zone&,
        Clock::time_point
    )>;
    using IdentityBindingResolver = std::function<std::optional<domain::TrackIdentityBinding>(
        const std::string&,
        const std::string&,
        Clock::time_point
    )>;

    XProtectLineCrossingService(
        ZoneResolver zoneResolver,
        ActivePresenceResolver activePresenceResolver
    );
    XProtectLineCrossingService(
        ZoneResolver zoneResolver,
        IdentityBindingResolver identityBindingResolver
    );

    XProtectLineCrossingDecision evaluate(const XProtectLineCrossingCommand& command) const;

private:
    ZoneResolver zoneResolver_;
    ActivePresenceResolver activePresenceResolver_;
    IdentityBindingResolver identityBindingResolver_;
};

}
