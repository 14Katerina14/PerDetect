#pragma once

#include "securezone/auth/IAccessTokenService.h"
#include "securezone/auth/IPasswordVerifier.h"
#include "securezone/repository/IAppUserRepository.h"

#include <chrono>
#include <functional>
#include <string>

namespace securezone::auth {

class AuthenticationService {
public:
    using Clock = IAccessTokenService::Clock;
    using NowProvider = std::function<Clock::time_point()>;

    AuthenticationService(
        const repository::IAppUserRepository& appUsers,
        const IPasswordVerifier& passwordVerifier,
        const IAccessTokenService& accessTokenService,
        std::string dummyPasswordHash,
        NowProvider nowProvider = [] { return Clock::now(); }
    );

    LoginResult login(const LoginCommand& command) const;

private:
    const repository::IAppUserRepository& appUsers_;
    const IPasswordVerifier& passwordVerifier_;
    const IAccessTokenService& accessTokenService_;
    std::string dummyPasswordHash_;
    NowProvider nowProvider_;
};

}
