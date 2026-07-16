#pragma once

#include "securezone/domain/PushNotification.h"

#include <optional>
#include <string>
#include <vector>

namespace securezone::repository {

class IPushSubscriptionRepository {
public:
    virtual ~IPushSubscriptionRepository() = default;

    virtual std::vector<domain::PushSubscription> findActiveManagerAndAdminSubscriptions() const = 0;
    virtual std::optional<domain::PushSubscription> findActiveBySubscriptionId(
        const std::string& subscriptionId
    ) const = 0;
};

}
