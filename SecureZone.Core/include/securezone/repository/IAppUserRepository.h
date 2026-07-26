#pragma once

#include <optional>
#include <string>

#include "securezone/domain/AppUser.h"

namespace securezone::repository {

class IAppUserRepository {
public:
    virtual ~IAppUserRepository() = default;

    virtual std::optional<domain::AppUser> findByUserId(
        const std::string& userId
    ) const = 0;

    virtual std::optional<domain::AppUser> findByUsername(
        const std::string& username
    ) const = 0;
};

}
