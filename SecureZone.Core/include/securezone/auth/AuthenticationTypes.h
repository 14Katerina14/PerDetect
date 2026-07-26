#pragma once

#include "securezone/domain/AppUser.h"

#include <chrono>
#include <optional>
#include <string>

namespace securezone::auth {

struct AuthenticatedPrincipal {
    std::string userId;
    std::string username;
    domain::AppUserRole role{domain::AppUserRole::Worker};
    std::string employeeId;
};

struct LoginCommand {
    std::string username;
    std::string password;
};

struct IssuedAccessToken {
    std::string value;
    std::chrono::seconds expiresIn{};
};

enum class LoginStatus {
    Success,
    InvalidCredentials,
    ServiceUnavailable
};

struct LoginResult {
    LoginStatus status{LoginStatus::InvalidCredentials};
    IssuedAccessToken accessToken;
    std::optional<AuthenticatedPrincipal> principal;

    bool succeeded() const {
        return status == LoginStatus::Success
            && principal.has_value()
            && !accessToken.value.empty();
    }
};

enum class AccessTokenValidationStatus {
    Valid,
    Invalid,
    Expired
};

struct AccessTokenValidationResult {
    AccessTokenValidationStatus status{AccessTokenValidationStatus::Invalid};
    std::optional<AuthenticatedPrincipal> principal;

    bool valid() const {
        return status == AccessTokenValidationStatus::Valid && principal.has_value();
    }
};

}
