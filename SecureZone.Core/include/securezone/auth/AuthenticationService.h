#pragma once

#include "securezone/auth/IAccessTokenService.h"
#include "securezone/auth/IPasswordVerifier.h"
#include "securezone/repository/IAppUserRepository.h"

#include <chrono>
#include <functional>

namespace securezone::auth {

class AuthenticationService {
public:
    using Clock = IAccessTokenService::Clock;
    using NowProvider = std::function<Clock::time_point()>;

    AuthenticationService(
        const repository::IAppUserRepository& appUsers,
        const IPasswordVerifier& passwordVerifier,
        const IAccessTokenService& accessTokenService,
        NowProvider nowProvider = [] { return Clock::now(); }
    );

    LoginResult login(const LoginCommand& command) const;

private:
    const repository::IAppUserRepository& appUsers_;
    const IPasswordVerifier& passwordVerifier_;
    const IAccessTokenService& accessTokenService_;
    NowProvider nowProvider_;
};

}
