#pragma once

#include "securezone/domain/PushNotification.h"

#include <chrono>
#include <cstddef>
#include <string>
#include <vector>

namespace securezone::repository {

class IPushNotificationDeliveryRepository {
public:
    virtual ~IPushNotificationDeliveryRepository() = default;

    virtual bool createIfAbsent(const domain::PushNotificationDelivery& delivery) = 0;
    virtual std::vector<domain::PushNotificationDelivery> findReadyForAttempt(
        std::chrono::system_clock::time_point now,
        std::size_t limit
    ) const = 0;
    virtual void markDelivered(
        const std::string& deliveryId,
        int attempts,
        std::chrono::system_clock::time_point attemptedAt,
        int responseCode
    ) = 0;
    virtual void markFailed(
        const std::string& deliveryId,
        int attempts,
        std::chrono::system_clock::time_point attemptedAt,
        std::chrono::system_clock::time_point nextAttemptAt,
        int responseCode,
        const std::string& error,
        bool exhausted
    ) = 0;
};

}
