#include "securezone/infrastructure/auth/JwtAccessTokenService.h"

#include "securezone/domain/AppUser.h"

#include <jwt-cpp/jwt.h>
#include <sodium.h>

#include <array>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>

namespace securezone::infrastructure::auth {
namespace {

constexpr std::size_t MinimumSecretBytes = 32;

securezone::auth::AccessTokenValidationResult invalid() {
    return {securezone::auth::AccessTokenValidationStatus::Invalid, std::nullopt};
}

securezone::auth::AccessTokenValidationResult expired() {
    return {securezone::auth::AccessTokenValidationStatus::Expired, std::nullopt};
}

std::string createJti() {
    std::array<unsigned char, 16> bytes{};
    randombytes_buf(bytes.data(), bytes.size());
    std::array<char, 33> encoded{};
    sodium_bin2hex(encoded.data(), encoded.size(), bytes.data(), bytes.size());
    return encoded.data();
}

template <typename DecodedToken>
bool hasRequiredClaims(const DecodedToken& token) {
    return token.has_subject()
        && token.has_issuer()
        && token.has_audience()
        && token.has_issued_at()
        && token.has_expires_at()
        && token.has_id()
        && token.has_payload_claim("preferred_username")
        && token.has_payload_claim("role");
}

}

JwtAccessTokenService::JwtAccessTokenService(JwtAccessTokenSettings settings)
    : settings_{std::move(settings)} {
    if (settings_.secret.size() < MinimumSecretBytes) {
        throw std::invalid_argument("SECUREZONE_JWT_SECRET must contain at least 32 bytes.");
    }
    if (settings_.ttl.count() <= 0) {
        throw std::invalid_argument("SECUREZONE_JWT_TTL_MINUTES must be positive.");
    }
    if (settings_.issuer.empty() || settings_.audience.empty()) {
        throw std::invalid_argument("JWT issuer and audience must not be empty.");
    }
    if (sodium_init() < 0) {
        throw std::runtime_error("libsodium initialization failed.");
    }
}

securezone::auth::IssuedAccessToken JwtAccessTokenService::issue(
    const securezone::auth::AuthenticatedPrincipal& principal,
    Clock::time_point issuedAt
) const {
    if (principal.userId.empty() || principal.username.empty()) {
        throw std::invalid_argument("JWT principal is missing required identity fields.");
    }

    auto builder = jwt::create()
        .set_type("JWT")
        .set_issuer(settings_.issuer)
        .set_audience(settings_.audience)
        .set_subject(principal.userId)
        .set_issued_at(issuedAt)
        .set_expires_at(issuedAt + settings_.ttl)
        .set_id(createJti())
        .set_payload_claim("preferred_username", jwt::claim(principal.username))
        .set_payload_claim("role", jwt::claim(std::string{domain::toString(principal.role)}));

    if (!principal.employeeId.empty()) {
        builder.set_payload_claim("employee_id", jwt::claim(principal.employeeId));
    }

    return {
        builder.sign(jwt::algorithm::hs256{settings_.secret}),
        std::chrono::duration_cast<std::chrono::seconds>(settings_.ttl)
    };
}

securezone::auth::AccessTokenValidationResult JwtAccessTokenService::validate(
    std::string_view token,
    Clock::time_point now
) const {
    if (token.empty()) {
        return invalid();
    }

    try {
        const auto decoded = jwt::decode(std::string{token});
        if (decoded.get_algorithm() != "HS256" || !hasRequiredClaims(decoded)) {
            return invalid();
        }
        if (decoded.get_expires_at() <= now) {
            return expired();
        }
        if (decoded.get_issued_at() > now + std::chrono::minutes{1}) {
            return invalid();
        }

        jwt::verify()
            .allow_algorithm(jwt::algorithm::hs256{settings_.secret})
            .with_issuer(settings_.issuer)
            .with_audience(settings_.audience)
            .verify(decoded);

        const auto role = domain::appUserRoleFromString(
            decoded.get_payload_claim("role").as_string()
        );
        if (!role.has_value()) {
            return invalid();
        }

        securezone::auth::AuthenticatedPrincipal principal{};
        principal.userId = decoded.get_subject();
        principal.username = decoded.get_payload_claim("preferred_username").as_string();
        principal.role = *role;
        if (decoded.has_payload_claim("employee_id")) {
            principal.employeeId = decoded.get_payload_claim("employee_id").as_string();
        }

        if (principal.userId.empty()
            || principal.username.empty()
            || (principal.role == domain::AppUserRole::Worker && principal.employeeId.empty())) {
            return invalid();
        }

        return {securezone::auth::AccessTokenValidationStatus::Valid, principal};
    } catch (...) {
        return invalid();
    }
}

}
