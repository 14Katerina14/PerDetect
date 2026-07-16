#pragma once

#include "securezone/api/http/HttpRequest.h"
#include "securezone/auth/AuthorizationPolicy.h"
#include "securezone/auth/IAccessTokenService.h"

#include <functional>
#include <optional>

namespace securezone::api {

enum class EndpointAuthorizationStatus {
    Authorized,
    Unauthorized,
    Forbidden
};

struct EndpointAuthorizationResult {
    EndpointAuthorizationStatus status{EndpointAuthorizationStatus::Unauthorized};
    std::optional<auth::AuthenticatedPrincipal> principal;
};

class EndpointAuthorizer {
public:
    using Clock = auth::IAccessTokenService::Clock;
    using NowProvider = std::function<Clock::time_point()>;
    using Handler = std::function<EndpointAuthorizationResult(
        const HttpRequest&,
        auth::AuthorizationPolicy
    )>;

    explicit EndpointAuthorizer(
        const auth::IAccessTokenService& accessTokens,
        NowProvider nowProvider = [] { return Clock::now(); }
    );

    EndpointAuthorizationResult authorize(
        const HttpRequest& request,
        auth::AuthorizationPolicy policy
    ) const;

private:
    const auth::IAccessTokenService& accessTokens_;
    NowProvider nowProvider_;
};

}
