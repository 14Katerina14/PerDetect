#include "securezone/api/auth/EndpointAuthorizer.h"

#include "securezone/api/http/HttpHeaders.h"

#include <algorithm>
#include <cctype>
#include <string>
#include <utility>

namespace securezone::api {
namespace {

std::optional<std::string> bearerToken(const HttpRequest& request) {
    const auto authorization = findHeaderValue(request, "Authorization");
    if (!authorization.has_value()) {
        return std::nullopt;
    }

    const auto separator = authorization->find(' ');
    if (separator == std::string::npos
        || separator == 0
        || separator + 1 >= authorization->size()
        || authorization->find(' ', separator + 1) != std::string::npos) {
        return std::nullopt;
    }

    auto scheme = authorization->substr(0, separator);
    std::transform(scheme.begin(), scheme.end(), scheme.begin(), [](char value) {
        return static_cast<char>(std::tolower(static_cast<unsigned char>(value)));
    });
    if (scheme != "bearer") {
        return std::nullopt;
    }

    return authorization->substr(separator + 1);
}

}

EndpointAuthorizer::EndpointAuthorizer(
    const auth::IAccessTokenService& accessTokens,
    NowProvider nowProvider
) : accessTokens_{accessTokens},
    nowProvider_{std::move(nowProvider)} {
}

EndpointAuthorizationResult EndpointAuthorizer::authorize(
    const HttpRequest& request,
    auth::AuthorizationPolicy policy
) const {
    const auto token = bearerToken(request);
    if (!token.has_value()) {
        return {};
    }

    const auto validation = accessTokens_.validate(*token, nowProvider_());
    if (!validation.valid()) {
        return {};
    }

    if (!auth::isAuthorized(*validation.principal, policy)) {
        return {EndpointAuthorizationStatus::Forbidden, validation.principal};
    }

    return {EndpointAuthorizationStatus::Authorized, validation.principal};
}

}
