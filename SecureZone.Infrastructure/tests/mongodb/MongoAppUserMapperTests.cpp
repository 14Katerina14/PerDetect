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
    std::string status = "active"
) {
    bsoncxx::builder::basic::document document;
    document.append(
        bsoncxx::builder::basic::kvp("userId", userId),
        bsoncxx::builder::basic::kvp("username", username),
        bsoncxx::builder::basic::kvp("role", role),
        bsoncxx::builder::basic::kvp("status", status)
    );
    return document.extract();
}

void mapsScannerUser() {
    const auto document = appUserDocument();

    const auto user = mongodb::mapAppUserDocument(document.view());

    assert(user.has_value());
    assert(user->userId == "APP-SCANNER-001");
    assert(user->username == "scanner");
    assert(user->role == domain::AppUserRole::Scanner);
    assert(user->status == domain::AppUserStatus::Active);
}

void mapsEverySupportedRole() {
    const auto managerDocument = appUserDocument(
        "APP-MANAGER-001",
        "manager",
        "manager"
    );
    const auto adminDocument = appUserDocument(
        "APP-ADMIN-001",
        "admin",
        "admin"
    );

    const auto manager = mongodb::mapAppUserDocument(managerDocument.view());
    const auto admin = mongodb::mapAppUserDocument(adminDocument.view());

    assert(manager && manager->role == domain::AppUserRole::Manager);
    assert(admin && admin->role == domain::AppUserRole::Admin);
}

void mapsInactiveStatus() {
    const auto document = appUserDocument(
        "APP-SCANNER-002",
        "inactive-scanner",
        "scanner",
        "inactive"
    );

    const auto user = mongodb::mapAppUserDocument(document.view());

    assert(user && user->status == domain::AppUserStatus::Inactive);
}

void rejectsInvalidRole() {
    const auto document = appUserDocument(
        "APP-UNKNOWN-001",
        "unknown-role",
        "operator"
    );

    assert(!mongodb::mapAppUserDocument(document.view()).has_value());
}

void rejectsInvalidStatus() {
    const auto document = appUserDocument(
        "APP-SCANNER-003",
        "invalid-status",
        "scanner",
        "blocked"
    );

    assert(!mongodb::mapAppUserDocument(document.view()).has_value());
}

void rejectsMissingRequiredField() {
    bsoncxx::builder::basic::document document;
    document.append(
        bsoncxx::builder::basic::kvp("userId", "APP-SCANNER-004"),
        bsoncxx::builder::basic::kvp("username", "missing-role"),
        bsoncxx::builder::basic::kvp("status", "active")
    );
    const auto value = document.extract();

    assert(!mongodb::mapAppUserDocument(value.view()).has_value());
}

void rejectsEmptyRequiredField() {
    const auto document = appUserDocument("", "scanner");

    assert(!mongodb::mapAppUserDocument(document.view()).has_value());
}

}

int main() {
    mapsScannerUser();
    mapsEverySupportedRole();
    mapsInactiveStatus();
    rejectsInvalidRole();
    rejectsInvalidStatus();
    rejectsMissingRequiredField();
    rejectsEmptyRequiredField();
}
