#include "securezone/api/routes/AuthRoutes.h"

#include "securezone/api/ApiResponse.h"
#include "securezone/domain/AppUser.h"

#include <nlohmann/json.hpp>

#include <chrono>
#include <mutex>
#include <string>
#include <unordered_map>
#include <utility>

namespace securezone::api {

namespace {

constexpr std::size_t MaximumLoginBodyBytes = 4096;
constexpr std::size_t MaximumUsernameLength = 128;
constexpr std::size_t MaximumPasswordLength = 256;
constexpr int MaximumFailedAttempts = 5;
constexpr auto AttemptWindow = std::chrono::minutes{1};
constexpr auto BlockDuration = std::chrono::minutes{1};

}

struct AuthRoutes::RateLimitState {
    struct Entry {
        int failures{0};
        bool windowInitialized{false};
        Clock::time_point windowStartedAt{};
        Clock::time_point blockedUntil{};
        Clock::time_point lastSeenAt{};
    };

    bool isBlocked(const std::string& key, Clock::time_point now) {
        std::scoped_lock lock{mutex};
        prune(now);
        const auto match = entries.find(key);
        return match != entries.end() && match->second.blockedUntil > now;
    }

    bool recordFailure(const std::string& key, Clock::time_point now) {
        std::scoped_lock lock{mutex};
        prune(now);
        auto& entry = entries[key];
        entry.lastSeenAt = now;
        if (!entry.windowInitialized
            || now - entry.windowStartedAt >= AttemptWindow) {
            entry.windowInitialized = true;
            entry.windowStartedAt = now;
            entry.failures = 0;
        }

        ++entry.failures;
        if (entry.failures >= MaximumFailedAttempts) {
            entry.blockedUntil = now + BlockDuration;
            return true;
        }
        return false;
    }

    void recordSuccess(const std::string& key) {
        std::scoped_lock lock{mutex};
        entries.erase(key);
    }

private:
    void prune(Clock::time_point now) {
        for (auto entry = entries.begin(); entry != entries.end();) {
            if (entry->second.blockedUntil <= now
                && now - entry->second.lastSeenAt >= AttemptWindow + BlockDuration) {
                entry = entries.erase(entry);
            } else {
                ++entry;
            }
        }
    }

    std::mutex mutex;
    std::unordered_map<std::string, Entry> entries;
};

AuthRoutes::AuthRoutes(LoginHandler loginHandler, NowProvider nowProvider)
    : loginHandler_{std::move(loginHandler)},
      rateLimitState_{std::make_shared<RateLimitState>()},
      nowProvider_{std::move(nowProvider)} {
}

HttpResponse AuthRoutes::handleLogin(const HttpRequest& request) const {
    if (request.body.size() > MaximumLoginBodyBytes) {
        return jsonResponse(413, R"({"error":"request_too_large"})");
    }

    const auto body = nlohmann::json::parse(request.body, nullptr, false);
    if (body.is_discarded()
        || !body.is_object()
        || !body.contains("username")
        || !body["username"].is_string()
        || !body.contains("password")
        || !body["password"].is_string()) {
        return jsonResponse(400, R"({"error":"invalid_request"})");
    }

    auth::LoginCommand command{
        body["username"].get<std::string>(),
        body["password"].get<std::string>()
    };
    if (command.username.empty()
        || command.username.size() > MaximumUsernameLength
        || command.password.empty()
        || command.password.size() > MaximumPasswordLength) {
        return jsonResponse(400, R"({"error":"invalid_request"})");
    }

    const auto clientKey = (request.remoteAddress.empty() ? "unknown" : request.remoteAddress)
        + std::string{"\n"} + command.username;
    const auto now = nowProvider_ ? nowProvider_() : Clock::now();
    if (rateLimitState_ && rateLimitState_->isBlocked(clientKey, now)) {
        return jsonResponse(429, R"({"error":"too_many_requests"})");
    }

    if (!loginHandler_) {
        return jsonResponse(503, R"({"error":"authentication_unavailable"})");
    }

    const auto result = loginHandler_(command);
    if (result.status == auth::LoginStatus::InvalidCredentials) {
        if (rateLimitState_ && rateLimitState_->recordFailure(clientKey, now)) {
            return jsonResponse(429, R"({"error":"too_many_requests"})");
        }
        return jsonResponse(401, R"({"error":"invalid_credentials"})");
    }

    if (rateLimitState_) {
        rateLimitState_->recordSuccess(clientKey);
    }
    if (!result.succeeded()) {
        return jsonResponse(503, R"({"error":"authentication_unavailable"})");
    }

    const auto& principal = *result.principal;
    nlohmann::json response{
        {"accessToken", result.accessToken.value},
        {"tokenType", "Bearer"},
        {"expiresIn", result.accessToken.expiresIn.count()},
        {"user", {
            {"userId", principal.userId},
            {"username", principal.username},
            {"role", std::string{domain::toString(principal.role)}},
            {"employeeId", principal.employeeId}
        }}
    };
    return jsonResponse(200, response.dump());
}

}
