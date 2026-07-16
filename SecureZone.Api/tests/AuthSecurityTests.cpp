#include "securezone/api/ApiServer.h"
#include "securezone/api/auth/EndpointAuthorizer.h"
#include "securezone/api/routes/AuthRoutes.h"
#include "securezone/repository/IAppUserRepository.h"

#include <cassert>
#include <chrono>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {

using namespace securezone::api;
namespace auth = securezone::auth;
namespace domain = securezone::domain;

class AppUsers final : public securezone::repository::IAppUserRepository {
public:
    std::vector<domain::AppUser> users;

    std::optional<domain::AppUser> findByUserId(const std::string& userId) const override {
        for (const auto& user : users) {
            if (user.userId == userId) return user;
        }
        return std::nullopt;
    }

    std::optional<domain::AppUser> findByUsername(const std::string& username) const override {
        for (const auto& user : users) {
            if (user.username == username) return user;
        }
        return std::nullopt;
    }
};

domain::AppUser scannerUser() {
    return {"APP-SCANNER-001", "test-user", {}, "test-password-hash",
        domain::AppUserRole::Scanner, domain::AppUserStatus::Active};
}

class AccessTokens final : public auth::IAccessTokenService {
public:
    auth::IssuedAccessToken issue(
        const auth::AuthenticatedPrincipal&,
        Clock::time_point
    ) const override {
        return {};
    }

    auth::AccessTokenValidationResult validate(
        std::string_view token,
        Clock::time_point
    ) const override {
        if (token == "expired") {
            return {auth::AccessTokenValidationStatus::Expired, std::nullopt};
        }
        if (token == "scanner") {
            return valid("APP-SCANNER-001", domain::AppUserRole::Scanner, {});
        }
        if (token == "worker") {
            return valid("APP-WORKER-001", domain::AppUserRole::Worker, "EMP-001");
        }
        if (token == "manager") {
            return valid("APP-MANAGER-001", domain::AppUserRole::Manager, {});
        }
        return {};
    }

private:
    static auth::AccessTokenValidationResult valid(
        std::string userId,
        domain::AppUserRole role,
        std::string employeeId
    ) {
        return {
            auth::AccessTokenValidationStatus::Valid,
            auth::AuthenticatedPrincipal{
                std::move(userId), "test-user", role, std::move(employeeId)
            }
        };
    }
};

void loginUsesStrictJsonAndNeverReturnsPasswordMaterial() {
    ApiServer server{{}, ApiRouteHandlers{
        {}, {}, {},
        [](const auth::LoginCommand& command) {
            assert(command.username == "scanner");
            assert(command.password == "entered-password");
            return auth::LoginResult{
                auth::LoginStatus::Success,
                {"test-jwt", std::chrono::seconds{3600}},
                auth::AuthenticatedPrincipal{
                    "APP-SCANNER-001", "scanner", domain::AppUserRole::Scanner, {}
                }
            };
        },
        {}
    }};

    const auto malformed = server.handle({
        "POST", "/api/auth/login", "{\"username\":\"scanner\",", {}
    });
    const auto wrongType = server.handle({
        "POST", "/api/auth/login", "{\"username\":\"scanner\",\"password\":42}", {}
    });
    const auto success = server.handle({
        "POST", "/api/auth/login",
        R"({"username":"scanner","password":"entered-password"})", {}
    });

    assert(malformed.statusCode == 400);
    assert(wrongType.statusCode == 400);
    assert(success.statusCode == 200);
    assert(success.body.find(R"("accessToken":"test-jwt")") != std::string::npos);
    assert(success.body.find(R"("tokenType":"Bearer")") != std::string::npos);
    assert(success.body.find(R"("expiresIn":3600)") != std::string::npos);
    assert(success.body.find("password") == std::string::npos);
    assert(success.body.find("passwordHash") == std::string::npos);
}

void invalidCredentialsHaveOnePublicResponse() {
    ApiServer server{{}, ApiRouteHandlers{
        {}, {}, {},
        [](const auth::LoginCommand&) {
            return auth::LoginResult{auth::LoginStatus::InvalidCredentials, {}, std::nullopt};
        },
        {}
    }};

    const auto response = server.handle({
        "POST", "/api/auth/login", R"({"username":"unknown","password":"wrong"})", {}
    });

    assert(response.statusCode == 401);
    assert(response.body == R"({"error":"invalid_credentials"})");
}

void qrEndpointEnforcesScannerJwtAndTrustedSubject() {
    AccessTokens accessTokens;
    AppUsers appUsers;
    appUsers.users = {
        scannerUser(),
        {"APP-WORKER-001", "test-user", "EMP-001", "test-password-hash",
            domain::AppUserRole::Worker, domain::AppUserStatus::Active},
        {"APP-MANAGER-001", "test-user", {}, "test-password-hash",
            domain::AppUserRole::Manager, domain::AppUserStatus::Active}
    };
    EndpointAuthorizer authorizer{accessTokens, appUsers,
        [] { return auth::IAccessTokenService::Clock::time_point{}; }};
    bool called = false;
    ApiServer server{{}, ApiRouteHandlers{
        [&called](const securezone::qr::QrCheckInCommand& command) {
            called = true;
            assert(command.employeeId == "EMP-001");
            assert(command.scannedByUserId == "APP-SCANNER-001");
            return securezone::qr::QrCheckInResult{
                true, "started", "SESSION-001", {}, {}, {}
            };
        },
        {},
        {},
        {},
        [&authorizer](const HttpRequest& request, auth::AuthorizationPolicy policy) {
            return authorizer.authorize(request, policy);
        }
    }};
    const std::string body =
        R"({"employeeId":"EMP-001","zoneId":"ZONE-001","scannedByUserId":"SPOOFED"})";

    const auto missing = server.handle({"POST", "/api/qr/check-in", body, {}});
    const auto malformed = server.handle({
        "POST", "/api/qr/check-in", body, {{"Authorization", "Bearer"}}
    });
    const auto expired = server.handle({
        "POST", "/api/qr/check-in", body, {{"Authorization", "Bearer expired"}}
    });
    const auto worker = server.handle({
        "POST", "/api/qr/check-in", body, {{"Authorization", "Bearer worker"}}
    });
    const auto manager = server.handle({
        "POST", "/api/qr/check-in", body, {{"Authorization", "Bearer manager"}}
    });

    assert(missing.statusCode == 401);
    assert(malformed.statusCode == 401);
    assert(expired.statusCode == 401);
    assert(worker.statusCode == 403);
    assert(manager.statusCode == 403);
    assert(!called);

    const auto scanner = server.handle({
        "POST", "/api/qr/check-in", body, {{"authorization", "Bearer scanner"}}
    });
    assert(scanner.statusCode == 201);
    assert(called);
}

void authorizationUsesCurrentUserStatusAndRole() {
    AccessTokens accessTokens;
    AppUsers appUsers;
    appUsers.users = {scannerUser()};
    EndpointAuthorizer authorizer{accessTokens, appUsers,
        [] { return auth::IAccessTokenService::Clock::time_point{}; }};
    const HttpRequest request{"POST", "/api/qr/check-in", {},
        {{"Authorization", "Bearer scanner"}}};

    assert(authorizer.authorize(request, auth::AuthorizationPolicy::Scanner).status
        == EndpointAuthorizationStatus::Authorized);

    appUsers.users.front().status = domain::AppUserStatus::Inactive;
    assert(authorizer.authorize(request, auth::AuthorizationPolicy::Scanner).status
        == EndpointAuthorizationStatus::Unauthorized);

    appUsers.users.front().status = domain::AppUserStatus::Active;
    appUsers.users.front().role = domain::AppUserRole::Manager;
    assert(authorizer.authorize(request, auth::AuthorizationPolicy::Scanner).status
        == EndpointAuthorizationStatus::Unauthorized);

    appUsers.users.clear();
    assert(authorizer.authorize(request, auth::AuthorizationPolicy::Scanner).status
        == EndpointAuthorizationStatus::Unauthorized);
}

void loginRateLimitAndInputBoundsAreEnforced() {
    auto now = AuthRoutes::Clock::time_point{};
    AuthRoutes routes{
        [](const auth::LoginCommand&) {
            return auth::LoginResult{auth::LoginStatus::InvalidCredentials, {}, std::nullopt};
        },
        [&now] { return now; }
    };
    const std::string body = R"({"username":"scanner","password":"wrong-password"})";

    for (int attempt = 0; attempt < 4; ++attempt) {
        assert(routes.handleLogin({"POST", "/api/auth/login", body, {}, "10.0.0.1"}).statusCode == 401);
    }
    assert(routes.handleLogin({"POST", "/api/auth/login", body, {}, "10.0.0.1"}).statusCode == 429);
    assert(routes.handleLogin({"POST", "/api/auth/login", body, {}, "10.0.0.2"}).statusCode == 401);

    now += std::chrono::minutes{2};
    assert(routes.handleLogin({"POST", "/api/auth/login", body, {}, "10.0.0.1"}).statusCode == 401);

    assert(routes.handleLogin({"POST", "/api/auth/login", std::string(4097, 'x'), {}, "10.0.0.3"}).statusCode == 413);
    const std::string oversizedPassword = R"({"username":"scanner","password":")"
        + std::string(257, 'x') + R"("})";
    assert(routes.handleLogin({"POST", "/api/auth/login", oversizedPassword, {}, "10.0.0.3"}).statusCode == 400);
}

void healthRemainsPublic() {
    const ApiServer server{};
    assert(server.handle({"GET", "/health", {}, {}}).statusCode == 200);
}

void productionSettingsRequireStrongJwtSecret() {
    ApiSettings settings{};
    bool missingThrew = false;
    try {
        validateProductionApiSettings(settings);
    } catch (const std::runtime_error&) {
        missingThrew = true;
    }
    assert(missingThrew);

    settings.jwtSecret = "<required-generate-at-least-32-random-bytes>";
    bool placeholderThrew = false;
    try {
        validateProductionApiSettings(settings);
    } catch (const std::runtime_error&) {
        placeholderThrew = true;
    }
    assert(placeholderThrew);

    settings.jwtSecret = "a-development-secret-that-is-at-least-32-bytes";
    validateProductionApiSettings(settings);

    settings.jwtTtl = std::chrono::minutes{0};
    bool ttlThrew = false;
    try {
        validateProductionApiSettings(settings);
    } catch (const std::runtime_error&) {
        ttlThrew = true;
    }
    assert(ttlThrew);
}

}

int main() {
    loginUsesStrictJsonAndNeverReturnsPasswordMaterial();
    invalidCredentialsHaveOnePublicResponse();
    qrEndpointEnforcesScannerJwtAndTrustedSubject();
    authorizationUsesCurrentUserStatusAndRole();
    loginRateLimitAndInputBoundsAreEnforced();
    healthRemainsPublic();
    productionSettingsRequireStrongJwtSecret();
}
