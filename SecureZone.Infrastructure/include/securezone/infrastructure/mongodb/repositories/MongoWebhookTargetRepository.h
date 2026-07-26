#pragma once

#include <mongocxx/collection.hpp>

#include <optional>
#include <string>
#include <vector>

#include "securezone/domain/WebhookTarget.h"
#include "securezone/repository/IWebhookTargetRepository.h"

namespace securezone::infrastructure::mongodb::repositories {

class MongoWebhookTargetRepository final
    : public repository::IWebhookTargetRepository {
public:
    explicit MongoWebhookTargetRepository(
        mongocxx::collection webhookTargetsCollection
    );

    std::optional<domain::WebhookTarget> findByTargetId(
        const std::string& targetId
    ) const override;
    std::vector<domain::WebhookTarget> findActive() const override;
    bool save(const domain::WebhookTarget& target) override;

private:
    mutable mongocxx::collection webhookTargetsCollection_;
};

}
