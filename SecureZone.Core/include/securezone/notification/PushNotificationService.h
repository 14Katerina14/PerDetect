#pragma once

#include "securezone/domain/Alarm.h"
#include "securezone/repository/IPushNotificationDeliveryRepository.h"
#include "securezone/repository/IPushSubscriptionRepository.h"

#include <chrono>
#include <cstddef>

namespace securezone::notification {

class PushNotificationService {
public:
    PushNotificationService(
        repository::IPushSubscriptionRepository& subscriptionRepository,
        repository::IPushNotificationDeliveryRepository& deliveryRepository
    );

    std::size_t queueAlarmNotifications(
        const domain::Alarm& alarm,
        std::chrono::system_clock::time_point queuedAt
    );

    std::size_t queueAlarmResolvedNotifications(
        const domain::Alarm& alarm,
        std::chrono::system_clock::time_point queuedAt
    );

private:
    repository::IPushSubscriptionRepository& subscriptionRepository_;
    repository::IPushNotificationDeliveryRepository& deliveryRepository_;
};

}
