#pragma once

#include "securezone/auth/IAccessTokenService.h"

#include <chrono>
#include <string>

namespace securezone::infrastructure::auth {

struct JwtAccessTokenSettings {
    std::string secret;
    std::chrono::minutes ttl{60};
    std::string issuer{"securezone"};
    std::string audience{"securezone-mobile"};
};

class JwtAccessTokenService final : public securezone::auth::IAccessTokenService {
public:
    explicit JwtAccessTokenService(JwtAccessTokenSettings settings);

    securezone::auth::IssuedAccessToken issue(
        const securezone::auth::AuthenticatedPrincipal& principal,
        Clock::time_point issuedAt
    ) const override;

    securezone::auth::AccessTokenValidationResult validate(
        std::string_view token,
        Clock::time_point now
    ) const override;

private:
    JwtAccessTokenSettings settings_;
};

}
