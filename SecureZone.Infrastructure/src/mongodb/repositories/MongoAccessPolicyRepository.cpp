#include "securezone/infrastructure/mongodb/repositories/MongoAccessPolicyRepository.h"

#include <bsoncxx/builder/basic/array.hpp>
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>
#include <mongocxx/options/update.hpp>

#include <utility>

#include "securezone/infrastructure/mongodb/MongoDocumentMappers.h"

namespace securezone::infrastructure::mongodb::repositories {

namespace {

constexpr const char* ZoneIdField = "zoneId";

const char* machineStatusToString(domain::MachineStatus status) {
    switch (status) {
        case domain::MachineStatus::Running:
            return "running";
        case domain::MachineStatus::Stopped:
            return "stopped";
        case domain::MachineStatus::Maintenance:
            return "maintenance";
    }

    return "stopped";
}

bsoncxx::array::value toStringArray(const std::vector<std::string>& values) {
    bsoncxx::builder::basic::array array;
    for (const auto& value : values) {
        array.append(value);
    }
    return array.extract();
}

bsoncxx::array::value toMachineStatusArray(
    const std::vector<domain::MachineStatus>& statuses
) {
    bsoncxx::builder::basic::array array;
    for (const auto status : statuses) {
        array.append(machineStatusToString(status));
    }
    return array.extract();
}

bsoncxx::document::value toAccessPolicyDocument(
    const domain::AccessPolicy& policy
) {
    auto roles = toStringArray(policy.allowedRoles);
    auto statuses = toMachineStatusArray(policy.machineStatesAllowed);
    bsoncxx::builder::basic::document document;
    document.append(
        bsoncxx::builder::basic::kvp("policyId", policy.policyId),
        bsoncxx::builder::basic::kvp(ZoneIdField, policy.zoneId),
        bsoncxx::builder::basic::kvp("allowedRoles", roles.view()),
        bsoncxx::builder::basic::kvp("machineStatesAllowed", statuses.view())
    );
    return document.extract();
}

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

bool MongoAccessPolicyRepository::save(const domain::AccessPolicy& policy) {
    bsoncxx::builder::basic::document filter;
    filter.append(bsoncxx::builder::basic::kvp(ZoneIdField, policy.zoneId));

    auto policyDocument = toAccessPolicyDocument(policy);
    bsoncxx::builder::basic::document update;
    update.append(bsoncxx::builder::basic::kvp("$set", policyDocument.view()));

    mongocxx::options::update options;
    options.upsert(true);
    return accessPoliciesCollection_
        .update_one(filter.view(), update.view(), options)
        .has_value();
}

}
