#pragma once

#include "securezone/auth/AuthenticationTypes.h"

#include <chrono>
#include <string_view>

namespace securezone::auth {

class IAccessTokenService {
public:
    using Clock = std::chrono::system_clock;

    virtual ~IAccessTokenService() = default;

    virtual IssuedAccessToken issue(
        const AuthenticatedPrincipal& principal,
        Clock::time_point issuedAt
    ) const = 0;

    virtual AccessTokenValidationResult validate(
        std::string_view token,
        Clock::time_point now
    ) const = 0;
};

}
