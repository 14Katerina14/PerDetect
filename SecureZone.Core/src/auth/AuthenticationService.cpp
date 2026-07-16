#include "securezone/auth/AuthenticationService.h"

#include <utility>

namespace securezone::auth {
namespace {

LoginResult invalidCredentials() {
    return {LoginStatus::InvalidCredentials, {}, std::nullopt};
}

bool hasValidIdentityLink(const domain::AppUser& user) {
    return user.role != domain::AppUserRole::Worker || !user.employeeId.empty();
}

AuthenticatedPrincipal principalFrom(const domain::AppUser& user) {
    return {user.userId, user.username, user.role, user.employeeId};
}

}

AuthenticationService::AuthenticationService(
    const repository::IAppUserRepository& appUsers,
    const IPasswordVerifier& passwordVerifier,
    const IAccessTokenService& accessTokenService,
    NowProvider nowProvider
) : appUsers_{appUsers},
    passwordVerifier_{passwordVerifier},
    accessTokenService_{accessTokenService},
    nowProvider_{std::move(nowProvider)} {
}

LoginResult AuthenticationService::login(const LoginCommand& command) const {
    if (command.username.empty() || command.password.empty()) {
        return invalidCredentials();
    }

    const auto user = appUsers_.findByUsername(command.username);
    if (!user.has_value()
        || user->status != domain::AppUserStatus::Active
        || user->passwordHash.empty()
        || !hasValidIdentityLink(*user)) {
        return invalidCredentials();
    }

    bool verified = false;
    try {
        verified = passwordVerifier_.verify(command.password, user->passwordHash);
    } catch (...) {
        verified = false;
    }

    if (!verified) {
        return invalidCredentials();
    }

    const auto principal = principalFrom(*user);
    try {
        auto token = accessTokenService_.issue(principal, nowProvider_());
        if (token.value.empty() || token.expiresIn.count() <= 0) {
            return {LoginStatus::ServiceUnavailable, {}, std::nullopt};
        }
        return {LoginStatus::Success, std::move(token), principal};
    } catch (...) {
        return {LoginStatus::ServiceUnavailable, {}, std::nullopt};
    }
}

}
