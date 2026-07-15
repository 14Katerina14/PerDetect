#pragma once

#include <mongocxx/collection.hpp>

#include <optional>
#include <string>

#include "securezone/domain/AppUser.h"
#include "securezone/repository/IAppUserRepository.h"

namespace securezone::infrastructure::mongodb::repositories {

class MongoAppUserRepository final : public repository::IAppUserRepository {
public:
    explicit MongoAppUserRepository(mongocxx::collection appUsersCollection);

    std::optional<domain::AppUser> findByUserId(
        const std::string& userId
    ) const override;

    std::optional<domain::AppUser> findByUsername(
        const std::string& username
    ) const override;

private:
    mongocxx::collection appUsersCollection_;
};

}
