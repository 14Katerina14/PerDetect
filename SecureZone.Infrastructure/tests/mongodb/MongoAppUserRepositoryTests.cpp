#include <cassert>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>

#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>

#include "securezone/domain/AppUser.h"
#include "securezone/infrastructure/mongodb/repositories/MongoAppUserRepository.h"

namespace {

namespace domain = securezone::domain;
namespace repositories = securezone::infrastructure::mongodb::repositories;

constexpr int Skipped = 77;
constexpr const char* DatabaseName = "securezone_app_user_repository_tests";

bsoncxx::document::value appUserDocument(
    std::string userId,
    std::string username,
    std::string role,
    std::string status,
    std::string passwordHash = "test-hash-placeholder"
) {
    bsoncxx::builder::basic::document document;
    document.append(
        bsoncxx::builder::basic::kvp("userId", userId),
        bsoncxx::builder::basic::kvp("username", username),
        bsoncxx::builder::basic::kvp("role", role),
        bsoncxx::builder::basic::kvp("status", status),
        bsoncxx::builder::basic::kvp("passwordHash", passwordHash)
    );
    return document.extract();
}

}

int main() {
    const auto* mongoUri = std::getenv("SECUREZONE_MONGO_TEST_URI");
    if (mongoUri == nullptr || mongoUri[0] == '\0') {
        std::cout << "SKIP: SECUREZONE_MONGO_TEST_URI is not configured.\n";
        return Skipped;
    }

    mongocxx::instance instance{};
    mongocxx::client client{mongocxx::uri{mongoUri}};
    auto database = client[DatabaseName];
    auto collection = database["app_users"];
    bsoncxx::builder::basic::document emptyFilter;
    collection.delete_many(emptyFilter.view());

    const auto scannerDocument = appUserDocument(
        "APP-SCANNER-001",
        "scanner",
        "scanner",
        "active"
    );
    collection.insert_one(scannerDocument.view());

    repositories::MongoAppUserRepository repository{collection};

    const auto byUserId = repository.findByUserId("APP-SCANNER-001");
    assert(byUserId.has_value());
    assert(byUserId->username == "scanner");
    assert(byUserId->role == domain::AppUserRole::Scanner);
    assert(byUserId->status == domain::AppUserStatus::Active);

    const auto byUsername = repository.findByUsername("scanner");
    assert(byUsername.has_value());
    assert(byUsername->userId == "APP-SCANNER-001");
    assert(byUsername->role == domain::AppUserRole::Scanner);

    assert(!repository.findByUserId("APP-MISSING").has_value());
    assert(!repository.findByUsername("missing").has_value());

    const auto invalidRoleDocument = appUserDocument(
        "APP-INVALID-ROLE",
        "invalid-role",
        "operator",
        "active"
    );
    const auto invalidStatusDocument = appUserDocument(
        "APP-INVALID-STATUS",
        "invalid-status",
        "scanner",
        "blocked"
    );
    collection.insert_one(invalidRoleDocument.view());
    collection.insert_one(invalidStatusDocument.view());

    assert(!repository.findByUserId("APP-INVALID-ROLE").has_value());
    assert(!repository.findByUsername("invalid-status").has_value());

    database.drop();
}
