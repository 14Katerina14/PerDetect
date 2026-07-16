#include "securezone/auth/AuthenticationService.h"

#include <stdexcept>
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
    std::string dummyPasswordHash,
    NowProvider nowProvider
) : appUsers_{appUsers},
    passwordVerifier_{passwordVerifier},
    accessTokenService_{accessTokenService},
    dummyPasswordHash_{std::move(dummyPasswordHash)},
    nowProvider_{std::move(nowProvider)} {
    if (dummyPasswordHash_.empty()) {
        throw std::invalid_argument("A dummy password hash is required.");
    }
}

LoginResult AuthenticationService::login(const LoginCommand& command) const {
    if (command.username.empty() || command.password.empty()) {
        return invalidCredentials();
    }

    const auto user = appUsers_.findByUsername(command.username);
    const bool eligible = user.has_value()
        && user->status == domain::AppUserStatus::Active
        && !user->passwordHash.empty()
        && hasValidIdentityLink(*user);
    const std::string& passwordHash = eligible ? user->passwordHash : dummyPasswordHash_;

    bool verified = false;
    try {
        verified = passwordVerifier_.verify(command.password, passwordHash);
    } catch (...) {
        verified = false;
    }

    if (!eligible || !verified) {
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
