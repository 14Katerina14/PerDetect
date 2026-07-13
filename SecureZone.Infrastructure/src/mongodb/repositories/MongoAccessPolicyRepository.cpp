#include "securezone/infrastructure/mongodb/repositories/MongoAccessPolicyRepository.h"

#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>

#include <utility>

#include "securezone/infrastructure/mongodb/MongoDocumentMappers.h"

namespace securezone::infrastructure::mongodb::repositories {

namespace {

constexpr const char* ZoneIdField = "zoneId";

}

MongoAccessPolicyRepository::MongoAccessPolicyRepository(
    mongocxx::collection accessPoliciesCollection
) : accessPoliciesCollection_{std::move(accessPoliciesCollection)} {
}

std::optional<domain::AccessPolicy> MongoAccessPolicyRepository::findByZoneId(
    const std::string& zoneId
) const {
    bsoncxx::builder::basic::document filter;
    filter.append(bsoncxx::builder::basic::kvp(ZoneIdField, zoneId));

    auto result = accessPoliciesCollection_.find_one(filter.view());
    if (!result) {
        return std::nullopt;
    }

    return mapAccessPolicyDocument(result->view());
}

}
