#include "securezone/api/handlers/XProtectLineCrossingHandler.h"

#include <algorithm>
#include <cctype>
#include <ctime>
#include <iomanip>
#include <mutex>
#include <regex>
#include <sstream>
#include <unordered_map>
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

struct XProtectLineCrossingHandler::DecisionCache {
    std::mutex mutex;
    std::unordered_map<std::string, XProtectLineCrossingResult> decisions;
};

XProtectLineCrossingHandler::XProtectLineCrossingHandler(
    xprotect::XProtectLineCrossingService& service,
    ReceivedAtParser receivedAtParser
) : service_{service},
    receivedAtParser_{std::move(receivedAtParser)},
    decisionCache_{std::make_shared<DecisionCache>()} {
}

XProtectLineCrossingResult XProtectLineCrossingHandler::operator()(
    const XProtectLineCrossingEvent& event
) const {
    std::unique_lock<std::mutex> cacheLock;
    if (!event.eventId.empty()) {
        cacheLock = std::unique_lock<std::mutex>{decisionCache_->mutex};
        const auto cached = decisionCache_->decisions.find(event.eventId);
        if (cached != decisionCache_->decisions.end()) {
            auto duplicate = cached->second;
            duplicate.duplicate = true;
            return duplicate;
        }
    }

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

    auto result = toApiResult(service_.evaluate({
        event.eventName,
        event.sourceName,
        receivedAt,
        event.cameraId,
        event.objectId,
        event.action
    }));

    if (!event.eventId.empty() && result.accepted && result.status == "processed") {
        constexpr std::size_t MaxCachedDecisions = 4096;
        if (decisionCache_->decisions.size() >= MaxCachedDecisions) {
            decisionCache_->decisions.erase(decisionCache_->decisions.begin());
        }
        decisionCache_->decisions.emplace(event.eventId, result);
    }

    return result;
}

std::optional<XProtectLineCrossingHandler::Clock::time_point>
XProtectLineCrossingHandler::parseReceivedAt(const std::string& value) {
    if (value.empty()) {
        return Clock::time_point{};
    }

    if (isDigitsOnly(value)) {
        try {
            return Clock::time_point{std::chrono::milliseconds{std::stoll(value)}};
        } catch (const std::exception&) {
            return std::nullopt;
        }
    }

    static const std::regex IsoUtcPattern{
        R"(^(\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2})(?:\.(\d{1,9}))?Z$)"
    };
    std::smatch match;
    if (!std::regex_match(value, match, IsoUtcPattern)) {
        return std::nullopt;
    }

    std::tm time{};
    std::istringstream input{match[1].str()};
    input >> std::get_time(&time, "%Y-%m-%dT%H:%M:%S");
    if (input.fail()) {
        return std::nullopt;
    }

    const auto utc = utcTimeFromTm(time);
    if (!utc.has_value()) {
        return std::nullopt;
    }

    auto parsed = Clock::from_time_t(*utc);
    if (match[2].matched) {
        auto fractionalDigits = match[2].str();
        fractionalDigits.append(9 - fractionalDigits.size(), '0');
        parsed += std::chrono::duration_cast<Clock::duration>(
            std::chrono::nanoseconds{std::stoll(fractionalDigits)}
        );
    }

    return parsed;
}

}
