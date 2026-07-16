#include "securezone/api/ApiServer.h"
#include "securezone/api/auth/EndpointAuthorizer.h"

#include <cassert>
#include <chrono>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

namespace {

using namespace securezone::api;
namespace auth = securezone::auth;
namespace domain = securezone::domain;

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
    EndpointAuthorizer authorizer{accessTokens, [] { return auth::IAccessTokenService::Clock::time_point{}; }};
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
    healthRemainsPublic();
    productionSettingsRequireStrongJwtSecret();
}
