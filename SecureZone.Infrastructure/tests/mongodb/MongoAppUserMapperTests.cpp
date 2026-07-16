#include <cassert>
#include <string>

#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>

#include "securezone/domain/AppUser.h"
#include "securezone/infrastructure/mongodb/MongoDocumentMappers.h"

namespace {

namespace domain = securezone::domain;
namespace mongodb = securezone::infrastructure::mongodb;

bsoncxx::document::value appUserDocument(
    std::string userId = "APP-SCANNER-001",
    std::string username = "scanner",
    std::string role = "scanner",
    std::string status = "active",
    std::string passwordHash = "test-hash-placeholder",
    std::string employeeId = {}
) {
    bsoncxx::builder::basic::document document;
    document.append(
        bsoncxx::builder::basic::kvp("userId", userId),
        bsoncxx::builder::basic::kvp("username", username),
        bsoncxx::builder::basic::kvp("role", role),
        bsoncxx::builder::basic::kvp("status", status),
        bsoncxx::builder::basic::kvp("passwordHash", passwordHash)
    );
    if (!employeeId.empty()) {
        document.append(bsoncxx::builder::basic::kvp("employeeId", employeeId));
    }
    return document.extract();
}

void mapsScannerUserWithoutEmployee() {
    const auto mapped = mongodb::mapAppUserDocument(appUserDocument().view());

    assert(mapped.has_value());
    assert(mapped->userId == "APP-SCANNER-001");
    assert(mapped->username == "scanner");
    assert(mapped->passwordHash == "test-hash-placeholder");
    assert(mapped->employeeId.empty());
    assert(mapped->role == domain::AppUserRole::Scanner);
    assert(mapped->status == domain::AppUserStatus::Active);
}

void mapsWorkerWithEmployeeLink() {
    const auto mapped = mongodb::mapAppUserDocument(appUserDocument(
        "APP-WORKER-001", "worker", "worker", "active",
        "test-hash-placeholder", "EMP-001"
    ).view());

    assert(mapped.has_value());
    assert(mapped->role == domain::AppUserRole::Worker);
    assert(mapped->employeeId == "EMP-001");
}

void mapsManagerAdminAndInactiveStatus() {
    const auto manager = mongodb::mapAppUserDocument(
        appUserDocument("APP-MANAGER-001", "manager", "manager").view()
    );
    const auto admin = mongodb::mapAppUserDocument(
        appUserDocument("APP-ADMIN-001", "admin", "admin").view()
    );
    const auto inactive = mongodb::mapAppUserDocument(appUserDocument(
        "APP-SCANNER-002", "inactive-scanner", "scanner", "inactive"
    ).view());

    assert(manager && manager->role == domain::AppUserRole::Manager);
    assert(admin && admin->role == domain::AppUserRole::Admin);
    assert(inactive && inactive->status == domain::AppUserStatus::Inactive);
}

void malformedDocumentsFailClosed() {
    const auto invalidRole = mongodb::mapAppUserDocument(
        appUserDocument("U1", "invalid-role", "operator").view()
    );
    const auto invalidStatus = mongodb::mapAppUserDocument(
        appUserDocument("U2", "invalid-status", "scanner", "blocked").view()
    );
    const auto emptyUserId = mongodb::mapAppUserDocument(
        appUserDocument("", "scanner").view()
    );
    const auto missingHash = mongodb::mapAppUserDocument(
        appUserDocument("U3", "missing-hash", "scanner", "active", "").view()
    );
    const auto unlinkedWorker = mongodb::mapAppUserDocument(
        appUserDocument("U4", "unlinked-worker", "worker").view()
    );

    bsoncxx::builder::basic::document missingRoleDocument;
    missingRoleDocument.append(
        bsoncxx::builder::basic::kvp("userId", "U5"),
        bsoncxx::builder::basic::kvp("username", "missing-role"),
        bsoncxx::builder::basic::kvp("status", "active"),
        bsoncxx::builder::basic::kvp("passwordHash", "test-hash-placeholder")
    );
    const auto missingRoleValue = missingRoleDocument.extract();
    const auto missingRole = mongodb::mapAppUserDocument(missingRoleValue.view());

    bsoncxx::builder::basic::document wrongTypeDocument;
    wrongTypeDocument.append(
        bsoncxx::builder::basic::kvp("userId", 42),
        bsoncxx::builder::basic::kvp("username", "wrong-type"),
        bsoncxx::builder::basic::kvp("role", "scanner"),
        bsoncxx::builder::basic::kvp("status", "active"),
        bsoncxx::builder::basic::kvp("passwordHash", "test-hash-placeholder")
    );
    const auto wrongTypeValue = wrongTypeDocument.extract();
    const auto wrongType = mongodb::mapAppUserDocument(wrongTypeValue.view());

    assert(!invalidRole);
    assert(!invalidStatus);
    assert(!emptyUserId);
    assert(!missingHash);
    assert(!unlinkedWorker);
    assert(!missingRole);
    assert(!wrongType);
}

}

int main() {
    mapsScannerUserWithoutEmployee();
    mapsWorkerWithEmployeeLink();
    mapsManagerAdminAndInactiveStatus();
    malformedDocumentsFailClosed();
}
