#pragma once

#include <chrono>
#include <optional>
#include <string>

#include "securezone/webhook/WebhookNotificationService.h"

namespace securezone::webhook {

enum class WebhookDeliveryStatus { Pending, Delivered, Failed };

struct WebhookDeliveryRecord {
    std::string deliveryId;
    std::string alarmId;
    std::string targetUrl;
    AlarmNotificationPayload payload;
    WebhookDeliveryStatus status{WebhookDeliveryStatus::Pending};
    int attempts{};
    std::optional<std::chrono::system_clock::time_point> lastAttemptAt;
    std::optional<int> responseCode;
    std::chrono::system_clock::time_point createdAt{};
};

class WebhookDeliveryService {
public:
    WebhookDeliveryRecord createPendingDelivery(
        const AlarmNotificationPayload& payload,
        const std::string& targetUrl,
        std::chrono::system_clock::time_point createdAt
    ) const;
};

}
