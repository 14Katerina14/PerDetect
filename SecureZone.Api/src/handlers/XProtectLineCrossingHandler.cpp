#include "securezone/api/handlers/XProtectLineCrossingHandler.h"

#include <algorithm>
#include <cctype>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <utility>

namespace securezone::api {
namespace {

bool isDigitsOnly(const std::string& value) {
    return !value.empty()
        && std::all_of(value.begin(), value.end(), [](unsigned char character) {
            return std::isdigit(character) != 0;
        });
}

std::optional<std::time_t> utcTimeFromTm(std::tm& time) {
#if defined(_WIN32)
    const auto result = _mkgmtime(&time);
#else
    const auto result = timegm(&time);
#endif

    if (result == static_cast<std::time_t>(-1)) {
        return std::nullopt;
    }

    return result;
}

XProtectLineCrossingResult toApiResult(const xprotect::XProtectLineCrossingDecision& decision) {
    return XProtectLineCrossingResult{
        decision.accepted,
        decision.status,
        decision.decision,
        decision.zoneId,
        decision.sessionId,
        decision.employeeId,
        decision.message
    };
}

}

XProtectLineCrossingHandler::XProtectLineCrossingHandler(
    xprotect::XProtectLineCrossingService& service,
    ReceivedAtParser receivedAtParser
) : service_{service},
    receivedAtParser_{std::move(receivedAtParser)} {
}

XProtectLineCrossingResult XProtectLineCrossingHandler::operator()(
    const XProtectLineCrossingEvent& event
) const {
    auto receivedAt = Clock::time_point{};
    if (!event.receivedAt.empty()) {
        const auto parsed = receivedAtParser_(event.receivedAt);
        if (!parsed.has_value()) {
            return XProtectLineCrossingResult{
                false,
                "invalid_request",
                "none",
                {},
                {},
                {},
                "receivedAt must be ISO UTC or epoch milliseconds."
            };
        }
        receivedAt = *parsed;
    }

    return toApiResult(service_.evaluate({
        event.eventName,
        event.sourceName,
        receivedAt
    }));
}

std::optional<XProtectLineCrossingHandler::Clock::time_point>
XProtectLineCrossingHandler::parseReceivedAt(const std::string& value) {
    if (value.empty()) {
        return Clock::time_point{};
    }

    if (isDigitsOnly(value)) {
        return Clock::time_point{std::chrono::milliseconds{std::stoll(value)}};
    }

    std::tm time{};
    std::istringstream input{value};
    input >> std::get_time(&time, "%Y-%m-%dT%H:%M:%SZ");
    if (input.fail()) {
        return std::nullopt;
    }

    const auto utc = utcTimeFromTm(time);
    if (!utc.has_value()) {
        return std::nullopt;
    }

    return Clock::from_time_t(*utc);
}

}
