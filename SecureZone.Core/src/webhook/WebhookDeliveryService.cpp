#include "securezone/webhook/WebhookDeliveryService.h"

#include <chrono>
#include <string>

namespace securezone::webhook {

namespace {

std::string createDeliveryId(
    const std::string& alarmId,
    std::chrono::system_clock::time_point createdAt
) {
    const auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(
        createdAt.time_since_epoch()
    ).count();

    return "DELIVERY-" + alarmId + "-" + std::to_string(milliseconds);
}

}

WebhookDeliveryRecord WebhookDeliveryService::createPendingDelivery(
    const AlarmNotificationPayload& payload,
    const std::string& targetUrl,
    std::chrono::system_clock::time_point createdAt
) const {
    WebhookDeliveryRecord delivery{};
    delivery.deliveryId = createDeliveryId(payload.alarmId, createdAt);
    delivery.alarmId = payload.alarmId;
    delivery.targetUrl = targetUrl;
    delivery.payload = payload;
    delivery.status = WebhookDeliveryStatus::Pending;
    delivery.attempts = 0;
    delivery.createdAt = createdAt;
    return delivery;
}

}
