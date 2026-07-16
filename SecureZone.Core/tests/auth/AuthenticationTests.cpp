#include "securezone/auth/AuthenticationService.h"
#include "securezone/auth/AuthorizationPolicy.h"
#include "securezone/decision/AccessPolicyEvaluator.h"

#include <cassert>
#include <initializer_list>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>

namespace {

namespace auth = securezone::auth;
namespace domain = securezone::domain;
namespace repository = securezone::repository;

class AppUsers final : public repository::IAppUserRepository {
public:
    std::unordered_map<std::string, domain::AppUser> users;

    AppUsers() = default;
    AppUsers(std::initializer_list<std::pair<const std::string, domain::AppUser>> values)
        : users{values} {
    }

    std::optional<domain::AppUser> findByUserId(const std::string& userId) const override {
        for (const auto& [username, user] : users) {
            (void)username;
            if (user.userId == userId) return user;
        }
        return std::nullopt;
    }

    std::optional<domain::AppUser> findByUsername(const std::string& username) const override {
        const auto match = users.find(username);
        return match == users.end() ? std::nullopt : std::optional<domain::AppUser>{match->second};
    }
};

class PasswordVerifier final : public auth::IPasswordVerifier {
public:
    mutable int verificationCount{0};

    bool verify(std::string_view password, std::string_view hash) const override {
        ++verificationCount;
        return password == "correct-password" && hash == "valid-test-hash";
    }
};

class AccessTokens final : public auth::IAccessTokenService {
public:
    mutable std::optional<auth::AuthenticatedPrincipal> issuedFor;

    auth::IssuedAccessToken issue(
        const auth::AuthenticatedPrincipal& principal,
        Clock::time_point
    ) const override {
        issuedFor = principal;
        return {"test-access-token", std::chrono::seconds{3600}};
    }

    auth::AccessTokenValidationResult validate(std::string_view, Clock::time_point) const override {
        return {};
    }
};

domain::AppUser user(
    std::string userId,
    std::string username,
    domain::AppUserRole role,
    std::string employeeId = {},
    domain::AppUserStatus status = domain::AppUserStatus::Active,
    std::string passwordHash = "valid-test-hash"
) {
    return {std::move(userId), std::move(username), std::move(employeeId),
        std::move(passwordHash), role, status};
}

auth::AuthenticationService serviceFor(
    const AppUsers& users,
    const PasswordVerifier& passwords,
    const AccessTokens& tokens
) {
    return {users, passwords, tokens, "dummy-test-hash",
        [] { return auth::IAccessTokenService::Clock::time_point{}; }};
}

void scannerLoginSucceedsWithoutEmployeeLink() {
    AppUsers users{{{"scanner", user("APP-SCANNER-001", "scanner", domain::AppUserRole::Scanner)}}};
    PasswordVerifier passwords;
    AccessTokens tokens;

    const auto result = serviceFor(users, passwords, tokens).login({"scanner", "correct-password"});

    assert(result.succeeded());
    assert(result.principal->role == domain::AppUserRole::Scanner);
    assert(result.principal->employeeId.empty());
}

void workerLoginIncludesEmployeeId() {
    AppUsers users{{{"worker", user("APP-WORKER-001", "worker", domain::AppUserRole::Worker, "EMP-001")}}};
    PasswordVerifier passwords;
    AccessTokens tokens;

    const auto result = serviceFor(users, passwords, tokens).login({"worker", "correct-password"});

    assert(result.succeeded());
    assert(result.principal->employeeId == "EMP-001");
    assert(tokens.issuedFor->employeeId == "EMP-001");
}

void invalidCredentialsAreIndistinguishable() {
    PasswordVerifier passwords;
    AccessTokens tokens;
    AppUsers users{{
        {"inactive", user("U1", "inactive", domain::AppUserRole::Scanner, {}, domain::AppUserStatus::Inactive)},
        {"missing-hash", user("U2", "missing-hash", domain::AppUserRole::Scanner, {}, domain::AppUserStatus::Active, {})},
        {"worker-without-employee", user("U3", "worker-without-employee", domain::AppUserRole::Worker)}
    }};
    const auto service = serviceFor(users, passwords, tokens);

    const auto unknown = service.login({"unknown", "correct-password"});
    assert(passwords.verificationCount == 1);
    const auto wrong = service.login({"inactive", "wrong-password"});
    const auto inactive = service.login({"inactive", "correct-password"});
    const auto missingHash = service.login({"missing-hash", "correct-password"});
    const auto invalidWorker = service.login({"worker-without-employee", "correct-password"});

    for (const auto* result : {&unknown, &wrong, &inactive, &missingHash, &invalidWorker}) {
        assert(result->status == auth::LoginStatus::InvalidCredentials);
        assert(!result->principal.has_value());
        assert(result->accessToken.value.empty());
    }
}

void authorizationPoliciesKeepApplicationRolesSeparate() {
    const auth::AuthenticatedPrincipal worker{"U1", "worker", domain::AppUserRole::Worker, "EMP-001"};
    const auth::AuthenticatedPrincipal unlinkedWorker{"U2", "worker2", domain::AppUserRole::Worker, {}};
    const auth::AuthenticatedPrincipal scanner{"U3", "scanner", domain::AppUserRole::Scanner, {}};
    const auth::AuthenticatedPrincipal manager{"U4", "manager", domain::AppUserRole::Manager, {}};
    const auth::AuthenticatedPrincipal admin{"U5", "admin", domain::AppUserRole::Admin, {}};

    assert(auth::isAuthorized(worker, auth::AuthorizationPolicy::Worker));
    assert(!auth::isAuthorized(unlinkedWorker, auth::AuthorizationPolicy::Worker));
    assert(auth::isAuthorized(scanner, auth::AuthorizationPolicy::Scanner));
    assert(auth::isAuthorized(manager, auth::AuthorizationPolicy::ManagerOrAdmin));
    assert(auth::isAuthorized(admin, auth::AuthorizationPolicy::ManagerOrAdmin));
    assert(auth::isAuthorized(admin, auth::AuthorizationPolicy::AdminOnly));
    assert(!auth::isAuthorized(worker, auth::AuthorizationPolicy::Scanner));
}

void successfulWorkerLoginDoesNotGrantPhysicalZoneAccess() {
    AppUsers users{{{"worker", user(
        "APP-WORKER-001", "worker", domain::AppUserRole::Worker, "EMP-002"
    )}}};
    PasswordVerifier passwords;
    AccessTokens tokens;
    const auto login = serviceFor(users, passwords, tokens).login({"worker", "correct-password"});
    assert(login.succeeded());

    domain::Employee employee{};
    employee.employeeId = "EMP-002";
    employee.fullName = "Production worker";
    employee.roles = {"operator"};
    domain::AccessPolicy policy{};
    policy.allowedRoles = {"maintenance"};
    domain::MachineState machine{};
    machine.machineId = "MACHINE-001";
    machine.status = domain::MachineStatus::Stopped;

    const auto decision = securezone::decision::AccessPolicyEvaluator{}.evaluate(
        employee, machine, policy
    );
    assert(decision.type == domain::AccessDecisionType::Violation);
}

}

int main() {
    scannerLoginSucceedsWithoutEmployeeLink();
    workerLoginIncludesEmployeeId();
    invalidCredentialsAreIndistinguishable();
    authorizationPoliciesKeepApplicationRolesSeparate();
    successfulWorkerLoginDoesNotGrantPhysicalZoneAccess();
}
