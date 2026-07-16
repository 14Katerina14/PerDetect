#pragma once

#include <chrono>
#include <optional>
#include <string>

namespace securezone::domain {

enum class PushProvider { Expo, Fcm, Apns };
enum class PushSubscriptionStatus { Active, Inactive };
enum class PushDeliveryStatus { Pending, Delivered, Failed, Exhausted };

struct PushSubscription {
    std::string subscriptionId;
    std::string userId;
    PushProvider provider{PushProvider::Expo};
    std::string deviceToken;
    PushSubscriptionStatus status{PushSubscriptionStatus::Active};
    std::chrono::system_clock::time_point updatedAt{};
};

struct PushNotificationDelivery {
    std::string deliveryId;
    std::string alarmId;
    std::string subscriptionId;
    std::string recipientUserId;
    std::string title;
    std::string body;
    std::string zoneId;
    std::string employeeId;
    std::string reason;
    PushDeliveryStatus status{PushDeliveryStatus::Pending};
    int attempts{};
    std::chrono::system_clock::time_point createdAt{};
    std::chrono::system_clock::time_point nextAttemptAt{};
    std::optional<std::chrono::system_clock::time_point> lastAttemptAt;
    std::optional<int> responseCode;
    std::string lastError;
};

struct PushMessage {
    std::string title;
    std::string body;
    std::string alarmId;
    std::string zoneId;
    std::string employeeId;
    std::string reason;
};

}
