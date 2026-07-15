#include "securezone/infrastructure/mongodb/repositories/MongoAppUserRepository.h"

#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>

#include <utility>

#include "securezone/infrastructure/mongodb/MongoDocumentMappers.h"

namespace securezone::infrastructure::mongodb::repositories {

namespace {

constexpr const char* UserIdField = "userId";
constexpr const char* UsernameField = "username";

}

MongoAppUserRepository::MongoAppUserRepository(
    mongocxx::collection appUsersCollection
) : appUsersCollection_{std::move(appUsersCollection)} {
}

std::optional<domain::AppUser> MongoAppUserRepository::findByUserId(
    const std::string& userId
) const {
    bsoncxx::builder::basic::document filter;
    filter.append(bsoncxx::builder::basic::kvp(UserIdField, userId));

    auto result = appUsersCollection_.find_one(filter.view());
    if (!result) {
        return std::nullopt;
    }

    return mapAppUserDocument(result->view());
}

std::optional<domain::AppUser> MongoAppUserRepository::findByUsername(
    const std::string& username
) const {
    bsoncxx::builder::basic::document filter;
    filter.append(bsoncxx::builder::basic::kvp(UsernameField, username));

    auto result = appUsersCollection_.find_one(filter.view());
    if (!result) {
        return std::nullopt;
    }

    return mapAppUserDocument(result->view());
}

}
