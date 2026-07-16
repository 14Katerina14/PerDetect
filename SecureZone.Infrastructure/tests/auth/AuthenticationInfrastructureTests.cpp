#include "securezone/infrastructure/auth/JwtAccessTokenService.h"
#include "securezone/infrastructure/auth/SodiumPasswordVerifier.h"

#include <jwt-cpp/jwt.h>

#include <cassert>
#include <chrono>
#include <stdexcept>
#include <string>
#include <utility>

namespace {

namespace auth = securezone::auth;
namespace domain = securezone::domain;
namespace infrastructureAuth = securezone::infrastructure::auth;
using Clock = auth::IAccessTokenService::Clock;

const std::string TestSecret = "unit-test-secret-that-is-at-least-32-bytes-long";
const auto Now = Clock::now();

infrastructureAuth::JwtAccessTokenService tokens(
    std::string secret = TestSecret,
    std::string issuer = "securezone",
    std::string audience = "securezone-mobile"
) {
    return infrastructureAuth::JwtAccessTokenService{{
        std::move(secret), std::chrono::minutes{60}, std::move(issuer), std::move(audience)
    }};
}

auth::AuthenticatedPrincipal worker() {
    return {"APP-WORKER-001", "worker", domain::AppUserRole::Worker, "EMP-001"};
}

void argon2idHashVerifiesWithoutCommittedHash() {
    infrastructureAuth::SodiumPasswordVerifier verifier;
    const auto hash = infrastructureAuth::hashPasswordArgon2id("test-password");

    assert(hash.rfind("$argon2id$", 0) == 0);
    assert(verifier.verify("test-password", hash));
    assert(!verifier.verify("wrong-password", hash));
    assert(!verifier.verify("test-password", "malformed-hash"));
}

void validWorkerTokenContainsEmployeeClaim() {
    const auto service = tokens();
    const auto issued = service.issue(worker(), Now);
    const auto decoded = jwt::decode(issued.value);
    const auto validated = service.validate(issued.value, Now);

    assert(issued.expiresIn == std::chrono::seconds{3600});
    assert(decoded.get_algorithm() == "HS256");
    assert(decoded.get_subject() == "APP-WORKER-001");
    assert(decoded.get_payload_claim("employee_id").as_string() == "EMP-001");
    assert(validated.valid());
    assert(validated.principal->employeeId == "EMP-001");
}

void scannerTokenCanOmitEmployeeClaim() {
    const auth::AuthenticatedPrincipal scanner{
        "APP-SCANNER-001", "scanner", domain::AppUserRole::Scanner, {}
    };
    const auto service = tokens();
    const auto issued = service.issue(scanner, Now);
    const auto decoded = jwt::decode(issued.value);

    assert(!decoded.has_payload_claim("employee_id"));
    assert(service.validate(issued.value, Now).valid());
}

void rejectsExpiredAndWrongSignatureTokens() {
    auto service = tokens();
    const auto issued = service.issue(worker(), Now);

    assert(service.validate(issued.value, Now + std::chrono::minutes{61}).status
        == auth::AccessTokenValidationStatus::Expired);
    assert(tokens("another-unit-test-secret-at-least-32-bytes")
        .validate(issued.value, Now).status == auth::AccessTokenValidationStatus::Invalid);
}

void rejectsWrongIssuerAndAudience() {
    const auto wrongIssuer = tokens(TestSecret, "other-issuer").issue(worker(), Now);
    const auto wrongAudience = tokens(TestSecret, "securezone", "other-audience").issue(worker(), Now);
    const auto service = tokens();

    assert(!service.validate(wrongIssuer.value, Now).valid());
    assert(!service.validate(wrongAudience.value, Now).valid());
}

void rejectsUnsupportedAlgorithm() {
    const auto token = jwt::create()
        .set_type("JWT")
        .set_issuer("securezone")
        .set_audience("securezone-mobile")
        .set_subject("APP-WORKER-001")
        .set_issued_at(Now)
        .set_expires_at(Now + std::chrono::minutes{60})
        .set_id("unsupported-algorithm-test")
        .set_payload_claim("preferred_username", jwt::claim(std::string{"worker"}))
        .set_payload_claim("role", jwt::claim(std::string{"worker"}))
        .set_payload_claim("employee_id", jwt::claim(std::string{"EMP-001"}))
        .sign(jwt::algorithm::hs512{TestSecret});

    assert(!tokens().validate(token, Now).valid());
}

void rejectsMalformedTokenAndMissingClaims() {
    const auto missingRole = jwt::create()
        .set_type("JWT")
        .set_issuer("securezone")
        .set_audience("securezone-mobile")
        .set_subject("APP-WORKER-001")
        .set_issued_at(Now)
        .set_expires_at(Now + std::chrono::minutes{60})
        .set_id("missing-role-test")
        .set_payload_claim("preferred_username", jwt::claim(std::string{"worker"}))
        .sign(jwt::algorithm::hs256{TestSecret});

    assert(!tokens().validate("not-a-jwt", Now).valid());
    assert(!tokens().validate(missingRole, Now).valid());
}

void rejectsShortSecretConfiguration() {
    bool threw = false;
    try {
        (void)tokens("too-short");
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    assert(threw);
}

}

int main() {
    argon2idHashVerifiesWithoutCommittedHash();
    validWorkerTokenContainsEmployeeClaim();
    scannerTokenCanOmitEmployeeClaim();
    rejectsExpiredAndWrongSignatureTokens();
    rejectsWrongIssuerAndAudience();
    rejectsUnsupportedAlgorithm();
    rejectsMalformedTokenAndMissingClaims();
    rejectsShortSecretConfiguration();
}
