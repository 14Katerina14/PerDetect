#pragma once

#include "securezone/api/events/XProtectLineCrossingEvent.h"
#include "securezone/xprotect/XProtectLineCrossingService.h"

#include <chrono>
#include <functional>
#include <memory>
#include <optional>
#include <string>

namespace securezone::api {

class XProtectLineCrossingHandler {
public:
    using Clock = std::chrono::system_clock;
    using ReceivedAtParser = std::function<std::optional<Clock::time_point>(const std::string&)>;

    explicit XProtectLineCrossingHandler(
        xprotect::XProtectLineCrossingService& service,
        ReceivedAtParser receivedAtParser = parseReceivedAt
    );

    XProtectLineCrossingResult operator()(const XProtectLineCrossingEvent& event) const;

    static std::optional<Clock::time_point> parseReceivedAt(const std::string& value);

private:
    struct DecisionCache;

    xprotect::XProtectLineCrossingService& service_;
    ReceivedAtParser receivedAtParser_;
    std::shared_ptr<DecisionCache> decisionCache_;
};

}
