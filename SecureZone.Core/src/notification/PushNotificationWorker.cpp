#include "securezone/notification/PushNotificationWorker.h"

#include <algorithm>
#include <array>
#include <exception>
#include <string>

namespace securezone::notification {
namespace {

std::chrono::seconds retryDelay(int attempts) {
    constexpr std::array<std::chrono::seconds, 4> Delays{
        std::chrono::seconds{5},
        std::chrono::seconds{30},
        std::chrono::seconds{120},
        std::chrono::seconds{600}
    };
    const auto index = std::min<std::size_t>(
        attempts > 0 ? static_cast<std::size_t>(attempts - 1) : 0U,
        Delays.size() - 1U
    );
    return Delays[index];
}

domain::PushMessage messageFrom(const domain::PushNotificationDelivery& delivery) {
    return {
        delivery.title,
        delivery.body,
        delivery.alarmId,
        delivery.zoneId,
        delivery.employeeId,
        delivery.reason
    };
}

}

PushNotificationWorker::PushNotificationWorker(
    repository::IPushSubscriptionRepository& subscriptionRepository,
    repository::IPushNotificationDeliveryRepository& deliveryRepository,
    IPushNotificationProvider& provider,
    int maxAttempts
) : subscriptionRepository_{subscriptionRepository},
    deliveryRepository_{deliveryRepository},
    provider_{provider},
    maxAttempts_{std::max(1, maxAttempts)} {
}

std::size_t PushNotificationWorker::processReady(Clock::time_point now, std::size_t limit) {
    std::size_t processed{};
    for (const auto& delivery : deliveryRepository_.findReadyForAttempt(now, limit)) {
        const auto subscription = subscriptionRepository_.findActiveBySubscriptionId(
            delivery.subscriptionId
        );
        const auto attempts = delivery.attempts + 1;
        if (!subscription.has_value()) {
            deliveryRepository_.markFailed(
                delivery.deliveryId,
                attempts,
                now,
                now,
                0,
                "Push subscription is missing or inactive.",
                true
            );
            ++processed;
            continue;
        }

        PushSendResult sendResult{};
        try {
            sendResult = provider_.send(*subscription, messageFrom(delivery));
        } catch (const std::exception& exception) {
            sendResult.error = exception.what();
        } catch (...) {
            sendResult.error = "Push provider failed with an unknown error.";
        }

        if (sendResult.delivered) {
            deliveryRepository_.markDelivered(
                delivery.deliveryId,
                attempts,
                now,
                sendResult.responseCode
            );
        } else {
            const auto exhausted = attempts >= maxAttempts_;
            deliveryRepository_.markFailed(
                delivery.deliveryId,
                attempts,
                now,
                exhausted ? now : now + retryDelay(attempts),
                sendResult.responseCode,
                sendResult.error.empty() ? "Push provider rejected the notification." : sendResult.error,
                exhausted
            );
        }
        ++processed;
    }
    return processed;
}

}
