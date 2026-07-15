#include "securezone/infrastructure/mongodb/repositories/MongoWebhookTargetRepository.h"

#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>
#include <bsoncxx/types.hpp>
#include <mongocxx/options/update.hpp>

#include <chrono>
#include <utility>

#include "securezone/infrastructure/mongodb/MongoDocumentMappers.h"

namespace securezone::infrastructure::mongodb::repositories {

namespace {

constexpr const char* ActiveStatus = "active";
constexpr const char* StatusField = "status";
constexpr const char* TargetIdField = "targetId";

const char* statusToString(domain::WebhookTargetStatus status) {
    return status == domain::WebhookTargetStatus::Active
        ? ActiveStatus
        : "inactive";
}

bsoncxx::types::b_date toBsonDate(
    std::chrono::system_clock::time_point timePoint
) {
    return bsoncxx::types::b_date{
        std::chrono::duration_cast<std::chrono::milliseconds>(
            timePoint.time_since_epoch()
        )
    };
}

bsoncxx::document::value toDocument(const domain::WebhookTarget& target) {
    bsoncxx::builder::basic::document document;
    document.append(
        bsoncxx::builder::basic::kvp(TargetIdField, target.targetId),
        bsoncxx::builder::basic::kvp("name", target.name),
        bsoncxx::builder::basic::kvp("url", target.url),
        bsoncxx::builder::basic::kvp(StatusField, statusToString(target.status)),
        bsoncxx::builder::basic::kvp("createdAt", toBsonDate(target.createdAt)),
        bsoncxx::builder::basic::kvp("updatedAt", toBsonDate(target.updatedAt))
    );
    return document.extract();
}

}

MongoWebhookTargetRepository::MongoWebhookTargetRepository(
    mongocxx::collection webhookTargetsCollection
) : webhookTargetsCollection_{std::move(webhookTargetsCollection)} {
}

std::optional<domain::WebhookTarget>
MongoWebhookTargetRepository::findByTargetId(const std::string& targetId) const {
    bsoncxx::builder::basic::document filter;
    filter.append(bsoncxx::builder::basic::kvp(TargetIdField, targetId));

    const auto result = webhookTargetsCollection_.find_one(filter.view());
    if (!result) {
        return std::nullopt;
    }

    return mapWebhookTargetDocument(result->view());
}

std::vector<domain::WebhookTarget> MongoWebhookTargetRepository::findActive() const {
    bsoncxx::builder::basic::document filter;
    filter.append(bsoncxx::builder::basic::kvp(StatusField, ActiveStatus));

    std::vector<domain::WebhookTarget> targets;
    for (const auto& document : webhookTargetsCollection_.find(filter.view())) {
        targets.push_back(mapWebhookTargetDocument(document));
    }
    return targets;
}

bool MongoWebhookTargetRepository::save(const domain::WebhookTarget& target) {
    bsoncxx::builder::basic::document filter;
    filter.append(bsoncxx::builder::basic::kvp(TargetIdField, target.targetId));

    auto targetDocument = toDocument(target);
    bsoncxx::builder::basic::document update;
    update.append(bsoncxx::builder::basic::kvp("$set", targetDocument.view()));

    mongocxx::options::update options;
    options.upsert(true);
    return webhookTargetsCollection_
        .update_one(filter.view(), update.view(), options)
        .has_value();
}

}
