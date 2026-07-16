#pragma once

#include "securezone/notification/IPushNotificationProvider.h"
#include "securezone/repository/IPushNotificationDeliveryRepository.h"
#include "securezone/repository/IPushSubscriptionRepository.h"

#include <chrono>
#include <cstddef>

namespace securezone::notification {

class PushNotificationWorker {
public:
    using Clock = std::chrono::system_clock;

    PushNotificationWorker(
        repository::IPushSubscriptionRepository& subscriptionRepository,
        repository::IPushNotificationDeliveryRepository& deliveryRepository,
        IPushNotificationProvider& provider,
        int maxAttempts = 4
    );

    std::size_t processReady(Clock::time_point now, std::size_t limit = 100);

private:
    repository::IPushSubscriptionRepository& subscriptionRepository_;
    repository::IPushNotificationDeliveryRepository& deliveryRepository_;
    IPushNotificationProvider& provider_;
    int maxAttempts_;
};

}
