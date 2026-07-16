#pragma once

#include "securezone/repository/IPushNotificationDeliveryRepository.h"

#include <mongocxx/collection.hpp>

namespace securezone::infrastructure::mongodb::repositories {

class MongoPushNotificationDeliveryRepository final
    : public repository::IPushNotificationDeliveryRepository {
public:
    explicit MongoPushNotificationDeliveryRepository(mongocxx::collection collection);

    bool createIfAbsent(const domain::PushNotificationDelivery& delivery) override;
    std::vector<domain::PushNotificationDelivery> findReadyForAttempt(
        std::chrono::system_clock::time_point now,
        std::size_t limit
    ) const override;
    void markDelivered(
        const std::string& deliveryId,
        int attempts,
        std::chrono::system_clock::time_point attemptedAt,
        int responseCode
    ) override;
    void markFailed(
        const std::string& deliveryId,
        int attempts,
        std::chrono::system_clock::time_point attemptedAt,
        std::chrono::system_clock::time_point nextAttemptAt,
        int responseCode,
        const std::string& error,
        bool exhausted
    ) override;

private:
    mutable mongocxx::collection collection_;
};

}
