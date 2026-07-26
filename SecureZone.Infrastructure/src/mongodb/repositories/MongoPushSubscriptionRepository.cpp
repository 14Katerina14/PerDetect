#include "securezone/infrastructure/mongodb/repositories/MongoPushSubscriptionRepository.h"

#include <bsoncxx/builder/basic/array.hpp>
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>

#include <chrono>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace securezone::infrastructure::mongodb::repositories {
namespace {

using Clock = std::chrono::system_clock;
using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::make_document;

std::optional<std::string> stringField(bsoncxx::document::view document, const char* name) {
    const auto value = document[name];
    if (!value || value.type() != bsoncxx::type::k_string) return std::nullopt;
    return std::string{value.get_string().value};
}

std::optional<domain::PushProvider> providerFromString(const std::string& value) {
    if (value == "expo") return domain::PushProvider::Expo;
    if (value == "fcm") return domain::PushProvider::Fcm;
    if (value == "apns") return domain::PushProvider::Apns;
    return std::nullopt;
}

std::optional<domain::PushSubscription> mapSubscription(bsoncxx::document::view document) {
    const auto subscriptionId = stringField(document, "subscriptionId");
    const auto userId = stringField(document, "userId");
    const auto provider = stringField(document, "provider");
    const auto deviceToken = stringField(document, "deviceToken");
    const auto status = stringField(document, "status");
    if (!subscriptionId || !userId || !provider || !deviceToken || !status
        || *status != "active" || deviceToken->empty()) return std::nullopt;
    const auto mappedProvider = providerFromString(*provider);
    if (!mappedProvider.has_value()) return std::nullopt;

    domain::PushSubscription subscription{};
    subscription.subscriptionId = *subscriptionId;
    subscription.userId = *userId;
    subscription.provider = *mappedProvider;
    subscription.deviceToken = *deviceToken;
    subscription.status = domain::PushSubscriptionStatus::Active;
    const auto updatedAt = document["updatedAt"];
    if (updatedAt && updatedAt.type() == bsoncxx::type::k_date) {
        subscription.updatedAt = Clock::time_point{updatedAt.get_date().value};
    }
    return subscription;
}

}

MongoPushSubscriptionRepository::MongoPushSubscriptionRepository(
    mongocxx::collection appUsersCollection,
    mongocxx::collection pushSubscriptionsCollection
) : appUsersCollection_{std::move(appUsersCollection)},
    pushSubscriptionsCollection_{std::move(pushSubscriptionsCollection)} {
}

std::vector<domain::PushSubscription>
MongoPushSubscriptionRepository::findActiveManagerAndAdminSubscriptions() const {
    bsoncxx::builder::basic::array roles;
    roles.append("manager");
    roles.append("admin");
    bsoncxx::builder::basic::array userIds;
    std::size_t userIdCount{};
    for (const auto& user : appUsersCollection_.find(make_document(
        kvp("status", "active"),
        kvp("role", make_document(kvp("$in", roles)))
    ))) {
        const auto userId = stringField(user, "userId");
        if (userId && !userId->empty()) {
            userIds.append(*userId);
            ++userIdCount;
        }
    }

    if (userIdCount == 0U) return {};

    std::vector<domain::PushSubscription> subscriptions;
    for (const auto& document : pushSubscriptionsCollection_.find(make_document(
        kvp("status", "active"),
        kvp("userId", make_document(kvp("$in", userIds)))
    ))) {
        const auto subscription = mapSubscription(document);
        if (subscription) subscriptions.push_back(*subscription);
    }
    return subscriptions;
}

std::optional<domain::PushSubscription>
MongoPushSubscriptionRepository::findActiveBySubscriptionId(
    const std::string& subscriptionId
) const {
    const auto result = pushSubscriptionsCollection_.find_one(make_document(
        kvp("subscriptionId", subscriptionId),
        kvp("status", "active")
    ));
    return result ? mapSubscription(result->view()) : std::nullopt;
}

}
