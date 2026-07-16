#pragma once

#include "securezone/api/http/HttpRequest.h"
#include "securezone/api/http/HttpResponse.h"
#include "securezone/auth/AuthenticationTypes.h"

#include <chrono>
#include <functional>
#include <memory>

namespace securezone::api {

class AuthRoutes {
public:
    using LoginHandler = std::function<auth::LoginResult(const auth::LoginCommand&)>;
    using Clock = std::chrono::steady_clock;
    using NowProvider = std::function<Clock::time_point()>;

    AuthRoutes() = default;
    explicit AuthRoutes(
        LoginHandler loginHandler,
        NowProvider nowProvider = [] { return Clock::now(); }
    );

    HttpResponse handleLogin(const HttpRequest& request) const;

private:
    struct RateLimitState;

    LoginHandler loginHandler_;
    std::shared_ptr<RateLimitState> rateLimitState_;
    NowProvider nowProvider_;
};

}
