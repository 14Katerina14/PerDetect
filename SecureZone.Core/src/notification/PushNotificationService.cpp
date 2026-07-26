#include "securezone/notification/PushNotificationService.h"

#include <string>

namespace securezone::notification {
namespace {

std::string deliveryId(
    const domain::Alarm& alarm,
    const domain::PushSubscription& subscription,
    const std::string& lifecycle
) {
    return "PUSH-" + alarm.alarmId + "-" + subscription.subscriptionId + "-" + lifecycle;
}

std::string notificationBody(const domain::Alarm& alarm) {
    if (!alarm.message.empty()) return alarm.message;
    return "Access violation detected in zone " + alarm.zoneId + ".";
}

}

PushNotificationService::PushNotificationService(
    repository::IPushSubscriptionRepository& subscriptionRepository,
    repository::IPushNotificationDeliveryRepository& deliveryRepository
) : subscriptionRepository_{subscriptionRepository},
    deliveryRepository_{deliveryRepository} {
}

std::size_t PushNotificationService::queueAlarmNotifications(
    const domain::Alarm& alarm,
    std::chrono::system_clock::time_point queuedAt
) {
    std::size_t queued{};
    for (const auto& subscription : subscriptionRepository_.findActiveManagerAndAdminSubscriptions()) {
        domain::PushNotificationDelivery delivery{};
        delivery.deliveryId = deliveryId(alarm, subscription, "active");
        delivery.alarmId = alarm.alarmId;
        delivery.subscriptionId = subscription.subscriptionId;
        delivery.recipientUserId = subscription.userId;
        delivery.title = "SecureZone access violation";
        delivery.body = notificationBody(alarm);
        delivery.zoneId = alarm.zoneId;
        delivery.employeeId = alarm.employeeId;
        delivery.reason = alarm.reason;
        delivery.status = domain::PushDeliveryStatus::Pending;
        delivery.createdAt = queuedAt;
        delivery.nextAttemptAt = queuedAt;
        if (deliveryRepository_.createIfAbsent(delivery)) ++queued;
    }
    return queued;
}

std::size_t PushNotificationService::queueAlarmResolvedNotifications(
    const domain::Alarm& alarm,
    std::chrono::system_clock::time_point queuedAt
) {
    std::size_t queued{};
    for (const auto& subscription : subscriptionRepository_.findActiveManagerAndAdminSubscriptions()) {
        domain::PushNotificationDelivery delivery{};
        delivery.deliveryId = deliveryId(alarm, subscription, "resolved");
        delivery.alarmId = alarm.alarmId;
        delivery.subscriptionId = subscription.subscriptionId;
        delivery.recipientUserId = subscription.userId;
        delivery.title = "SecureZone violation cleared";
        delivery.body = "Access violation cleared in zone " + alarm.zoneId + ".";
        delivery.zoneId = alarm.zoneId;
        delivery.employeeId = alarm.employeeId;
        delivery.reason = alarm.reason;
        delivery.status = domain::PushDeliveryStatus::Pending;
        delivery.createdAt = queuedAt;
        delivery.nextAttemptAt = queuedAt;
        if (deliveryRepository_.createIfAbsent(delivery)) ++queued;
    }
    return queued;
}

}
