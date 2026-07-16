#pragma once

#include "securezone/repository/IPushSubscriptionRepository.h"

#include <mongocxx/collection.hpp>

namespace securezone::infrastructure::mongodb::repositories {

class MongoPushSubscriptionRepository final : public repository::IPushSubscriptionRepository {
public:
    MongoPushSubscriptionRepository(
        mongocxx::collection appUsersCollection,
        mongocxx::collection pushSubscriptionsCollection
    );

    std::vector<domain::PushSubscription> findActiveManagerAndAdminSubscriptions() const override;
    std::optional<domain::PushSubscription> findActiveBySubscriptionId(
        const std::string& subscriptionId
    ) const override;

private:
    mutable mongocxx::collection appUsersCollection_;
    mutable mongocxx::collection pushSubscriptionsCollection_;
};

}
