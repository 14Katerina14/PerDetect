#include "securezone/notification/PushNotificationService.h"
#include "securezone/notification/PushNotificationWorker.h"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

using namespace securezone;
using Clock = std::chrono::system_clock;

Clock::time_point at(int seconds) {
    return Clock::time_point{} + std::chrono::seconds{seconds};
}

class FakeSubscriptionRepository final : public repository::IPushSubscriptionRepository {
public:
    std::vector<domain::PushSubscription> subscriptions;

    std::vector<domain::PushSubscription> findActiveManagerAndAdminSubscriptions() const override {
        return subscriptions;
    }

    std::optional<domain::PushSubscription> findActiveBySubscriptionId(
        const std::string& subscriptionId
    ) const override {
        const auto found = std::find_if(
            subscriptions.begin(),
            subscriptions.end(),
            [&subscriptionId](const auto& subscription) {
                return subscription.subscriptionId == subscriptionId
                    && subscription.status == domain::PushSubscriptionStatus::Active;
            }
        );
        return found == subscriptions.end()
            ? std::nullopt
            : std::optional<domain::PushSubscription>{*found};
    }
};

class FakeDeliveryRepository final : public repository::IPushNotificationDeliveryRepository {
public:
    std::vector<domain::PushNotificationDelivery> deliveries;

    bool createIfAbsent(const domain::PushNotificationDelivery& delivery) override {
        const auto duplicate = std::any_of(
            deliveries.begin(),
            deliveries.end(),
            [&delivery](const auto& current) { return current.deliveryId == delivery.deliveryId; }
        );
        if (duplicate) return false;
        deliveries.push_back(delivery);
        return true;
    }

    std::vector<domain::PushNotificationDelivery> findReadyForAttempt(
        Clock::time_point now,
        std::size_t limit
    ) const override {
        std::vector<domain::PushNotificationDelivery> ready;
        for (const auto& delivery : deliveries) {
            if ((delivery.status == domain::PushDeliveryStatus::Pending
                    || delivery.status == domain::PushDeliveryStatus::Failed)
                && delivery.nextAttemptAt <= now) {
                ready.push_back(delivery);
                if (ready.size() == limit) break;
            }
        }
        return ready;
    }

    void markDelivered(
        const std::string& deliveryId,
        int attempts,
        Clock::time_point attemptedAt,
        int responseCode
    ) override {
        auto& delivery = find(deliveryId);
        delivery.status = domain::PushDeliveryStatus::Delivered;
        delivery.attempts = attempts;
        delivery.lastAttemptAt = attemptedAt;
        delivery.responseCode = responseCode;
    }

    void markFailed(
        const std::string& deliveryId,
        int attempts,
        Clock::time_point attemptedAt,
        Clock::time_point nextAttemptAt,
        int responseCode,
        const std::string& error,
        bool exhausted
    ) override {
        auto& delivery = find(deliveryId);
        delivery.status = exhausted
            ? domain::PushDeliveryStatus::Exhausted
            : domain::PushDeliveryStatus::Failed;
        delivery.attempts = attempts;
        delivery.lastAttemptAt = attemptedAt;
        delivery.nextAttemptAt = nextAttemptAt;
        delivery.responseCode = responseCode;
        delivery.lastError = error;
    }

private:
    domain::PushNotificationDelivery& find(const std::string& deliveryId) {
        const auto found = std::find_if(
            deliveries.begin(),
            deliveries.end(),
            [&deliveryId](const auto& delivery) { return delivery.deliveryId == deliveryId; }
        );
        assert(found != deliveries.end());
        return *found;
    }
};

class FakeProvider final : public notification::IPushNotificationProvider {
public:
    notification::PushSendResult result{true, 200, {}};
    bool throwError{};
    int calls{};
    domain::PushMessage lastMessage;

    notification::PushSendResult send(
        const domain::PushSubscription&,
        const domain::PushMessage& message
    ) override {
        ++calls;
        lastMessage = message;
        if (throwError) throw std::runtime_error{"provider unavailable"};
        return result;
    }
};

domain::PushSubscription subscription(std::string id, std::string userId) {
    return {std::move(id), std::move(userId), domain::PushProvider::Expo,
            "device-token", domain::PushSubscriptionStatus::Active, at(1)};
}

domain::Alarm alarm() {
    domain::Alarm value{};
    value.alarmId = "ALARM-001";
    value.zoneId = "ZONE-001";
    value.employeeId = "EMP-001";
    value.reason = "Role is not allowed.";
    value.message = "Ivan Petrov entered Machine A dangerous zone.";
    return value;
}

void queuesOneDeliveryPerRecipientAndDeduplicates() {
    FakeSubscriptionRepository subscriptions;
    subscriptions.subscriptions = {
        subscription("SUB-MANAGER", "USER-MANAGER"),
        subscription("SUB-ADMIN", "USER-ADMIN")
    };
    FakeDeliveryRepository deliveries;
    notification::PushNotificationService service{subscriptions, deliveries};

    assert(service.queueAlarmNotifications(alarm(), at(10)) == 2U);
    assert(service.queueAlarmNotifications(alarm(), at(11)) == 0U);
    assert(deliveries.deliveries.size() == 2U);
    assert(deliveries.deliveries.front().title == "SecureZone access violation");
    assert(deliveries.deliveries.front().body == alarm().message);
    assert(deliveries.deliveries.front().reason == alarm().reason);
}

void deliversQueuedNotification() {
    FakeSubscriptionRepository subscriptions;
    subscriptions.subscriptions = {subscription("SUB-1", "USER-1")};
    FakeDeliveryRepository deliveries;
    notification::PushNotificationService{subscriptions, deliveries}
        .queueAlarmNotifications(alarm(), at(10));
    FakeProvider provider;
    notification::PushNotificationWorker worker{subscriptions, deliveries, provider};

    assert(worker.processReady(at(10)) == 1U);
    assert(provider.calls == 1);
    assert(provider.lastMessage.alarmId == "ALARM-001");
    assert(deliveries.deliveries.front().status == domain::PushDeliveryStatus::Delivered);
    assert(deliveries.deliveries.front().attempts == 1);
}

void retriesProviderFailureAndEventuallyExhausts() {
    FakeSubscriptionRepository subscriptions;
    subscriptions.subscriptions = {subscription("SUB-1", "USER-1")};
    FakeDeliveryRepository deliveries;
    notification::PushNotificationService{subscriptions, deliveries}
        .queueAlarmNotifications(alarm(), at(10));
    FakeProvider provider;
    provider.result = {false, 503, "provider unavailable"};
    notification::PushNotificationWorker worker{subscriptions, deliveries, provider, 2};

    worker.processReady(at(10));
    assert(deliveries.deliveries.front().status == domain::PushDeliveryStatus::Failed);
    assert(deliveries.deliveries.front().nextAttemptAt == at(15));
    worker.processReady(at(15));
    assert(deliveries.deliveries.front().status == domain::PushDeliveryStatus::Exhausted);
    assert(deliveries.deliveries.front().attempts == 2);
}

void exhaustsDeliveryWhenSubscriptionIsInactive() {
    FakeSubscriptionRepository subscriptions;
    FakeDeliveryRepository deliveries;
    auto delivery = domain::PushNotificationDelivery{};
    delivery.deliveryId = "PUSH-1";
    delivery.subscriptionId = "MISSING";
    delivery.status = domain::PushDeliveryStatus::Pending;
    delivery.nextAttemptAt = at(10);
    deliveries.deliveries.push_back(delivery);
    FakeProvider provider;
    notification::PushNotificationWorker worker{subscriptions, deliveries, provider};

    worker.processReady(at(10));
    assert(provider.calls == 0);
    assert(deliveries.deliveries.front().status == domain::PushDeliveryStatus::Exhausted);
}

void convertsProviderExceptionsIntoRetryableFailures() {
    FakeSubscriptionRepository subscriptions;
    subscriptions.subscriptions = {subscription("SUB-1", "USER-1")};
    FakeDeliveryRepository deliveries;
    notification::PushNotificationService{subscriptions, deliveries}
        .queueAlarmNotifications(alarm(), at(10));
    FakeProvider provider;
    provider.throwError = true;
    notification::PushNotificationWorker worker{subscriptions, deliveries, provider};

    worker.processReady(at(10));
    assert(deliveries.deliveries.front().status == domain::PushDeliveryStatus::Failed);
    assert(deliveries.deliveries.front().lastError == "provider unavailable");
}

}

int main() {
    queuesOneDeliveryPerRecipientAndDeduplicates();
    deliversQueuedNotification();
    retriesProviderFailureAndEventuallyExhausts();
    exhaustsDeliveryWhenSubscriptionIsInactive();
    convertsProviderExceptionsIntoRetryableFailures();
}
