#pragma once

#include <mongocxx/collection.hpp>

#include <optional>
#include <string>

#include "securezone/domain/AccessPolicy.h"
#include "securezone/repository/IAccessPolicyRepository.h"

namespace securezone::infrastructure::mongodb::repositories {

class MongoAccessPolicyRepository final : public repository::IAccessPolicyRepository {
public:
    explicit MongoAccessPolicyRepository(mongocxx::collection accessPoliciesCollection);

    std::optional<domain::AccessPolicy> findByZoneId(
        const std::string& zoneId
    ) const override;

    bool save(const domain::AccessPolicy& policy) override;

private:
    mongocxx::collection accessPoliciesCollection_;
};

}
