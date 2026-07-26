#pragma once

#include <optional>
#include <string>
#include <vector>

#include "securezone/domain/WebhookTarget.h"

namespace securezone::repository {

class IWebhookTargetRepository {
public:
    virtual ~IWebhookTargetRepository() = default;

    virtual std::optional<domain::WebhookTarget> findByTargetId(
        const std::string& targetId
    ) const = 0;
    virtual std::vector<domain::WebhookTarget> findActive() const = 0;
    virtual bool save(const domain::WebhookTarget& target) = 0;
};

}
