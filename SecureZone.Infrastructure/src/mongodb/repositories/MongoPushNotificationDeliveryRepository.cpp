#include "securezone/infrastructure/mongodb/repositories/MongoPushNotificationDeliveryRepository.h"

#include <bsoncxx/builder/basic/array.hpp>
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>
#include <mongocxx/options/find.hpp>
#include <mongocxx/options/update.hpp>

#include <optional>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace securezone::infrastructure::mongodb::repositories {
namespace {

using Clock = std::chrono::system_clock;
using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::make_document;

bsoncxx::types::b_date date(Clock::time_point value) {
    return bsoncxx::types::b_date{value};
}

std::optional<std::string> stringField(bsoncxx::document::view document, const char* name) {
    const auto value = document[name];
    if (!value || value.type() != bsoncxx::type::k_string) return std::nullopt;
    return std::string{value.get_string().value};
}

std::optional<domain::PushDeliveryStatus> statusFromString(const std::string& value) {
    if (value == "pending") return domain::PushDeliveryStatus::Pending;
    if (value == "delivered") return domain::PushDeliveryStatus::Delivered;
    if (value == "failed") return domain::PushDeliveryStatus::Failed;
    if (value == "exhausted") return domain::PushDeliveryStatus::Exhausted;
    return std::nullopt;
}

std::optional<domain::PushNotificationDelivery> mapDelivery(bsoncxx::document::view document) {
    const auto deliveryId = stringField(document, "deliveryId");
    const auto alarmId = stringField(document, "alarmId");
    const auto subscriptionId = stringField(document, "subscriptionId");
    const auto recipientUserId = stringField(document, "recipientUserId");
    const auto title = stringField(document, "title");
    const auto body = stringField(document, "body");
    const auto status = stringField(document, "status");
    if (!deliveryId || !alarmId || !subscriptionId || !recipientUserId
        || !title || !body || !status) return std::nullopt;
    const auto mappedStatus = statusFromString(*status);
    if (!mappedStatus.has_value()) return std::nullopt;

    domain::PushNotificationDelivery delivery{};
    delivery.deliveryId = *deliveryId;
    delivery.alarmId = *alarmId;
    delivery.subscriptionId = *subscriptionId;
    delivery.recipientUserId = *recipientUserId;
    delivery.title = *title;
    delivery.body = *body;
    delivery.zoneId = stringField(document, "zoneId").value_or("");
    delivery.employeeId = stringField(document, "employeeId").value_or("");
    delivery.reason = stringField(document, "reason").value_or("");
    delivery.status = *mappedStatus;
    const auto attempts = document["attempts"];
    if (attempts && attempts.type() == bsoncxx::type::k_int32) delivery.attempts = attempts.get_int32().value;
    const auto createdAt = document["createdAt"];
    if (createdAt && createdAt.type() == bsoncxx::type::k_date) {
        delivery.createdAt = Clock::time_point{createdAt.get_date().value};
    }
    const auto nextAttemptAt = document["nextAttemptAt"];
    if (nextAttemptAt && nextAttemptAt.type() == bsoncxx::type::k_date) {
        delivery.nextAttemptAt = Clock::time_point{nextAttemptAt.get_date().value};
    }
    return delivery;
}

bsoncxx::document::value deliveryDocument(const domain::PushNotificationDelivery& delivery) {
    return make_document(
        kvp("deliveryId", delivery.deliveryId),
        kvp("alarmId", delivery.alarmId),
        kvp("subscriptionId", delivery.subscriptionId),
        kvp("recipientUserId", delivery.recipientUserId),
        kvp("title", delivery.title),
        kvp("body", delivery.body),
        kvp("zoneId", delivery.zoneId),
        kvp("employeeId", delivery.employeeId),
        kvp("reason", delivery.reason),
        kvp("status", "pending"),
        kvp("attempts", delivery.attempts),
        kvp("createdAt", date(delivery.createdAt)),
        kvp("nextAttemptAt", date(delivery.nextAttemptAt))
    );
}

}

MongoPushNotificationDeliveryRepository::MongoPushNotificationDeliveryRepository(
    mongocxx::collection collection
) : collection_{std::move(collection)} {
}

bool MongoPushNotificationDeliveryRepository::createIfAbsent(
    const domain::PushNotificationDelivery& delivery
) {
    mongocxx::options::update options;
    options.upsert(true);
    const auto document = deliveryDocument(delivery);
    const auto result = collection_.update_one(
        make_document(kvp("deliveryId", delivery.deliveryId)),
        make_document(kvp("$setOnInsert", document.view())),
        options
    );
    return result.has_value() && result->upserted_count() == 1U;
}

std::vector<domain::PushNotificationDelivery>
MongoPushNotificationDeliveryRepository::findReadyForAttempt(
    Clock::time_point now,
    std::size_t limit
) const {
    bsoncxx::builder::basic::array statuses;
    statuses.append("pending");
    statuses.append("failed");
    mongocxx::options::find options;
    options.sort(make_document(kvp("nextAttemptAt", 1)));
    options.limit(static_cast<std::int64_t>(limit));
    std::vector<domain::PushNotificationDelivery> deliveries;
    for (const auto& document : collection_.find(make_document(
        kvp("status", make_document(kvp("$in", statuses))),
        kvp("nextAttemptAt", make_document(kvp("$lte", date(now))))
    ), options)) {
        const auto delivery = mapDelivery(document);
        if (delivery) deliveries.push_back(*delivery);
    }
    return deliveries;
}

void MongoPushNotificationDeliveryRepository::markDelivered(
    const std::string& deliveryId,
    int attempts,
    Clock::time_point attemptedAt,
    int responseCode
) {
    collection_.update_one(
        make_document(kvp("deliveryId", deliveryId)),
        make_document(kvp("$set", make_document(
            kvp("status", "delivered"),
            kvp("attempts", attempts),
            kvp("lastAttemptAt", date(attemptedAt)),
            kvp("responseCode", responseCode),
            kvp("lastError", "")
        )))
    );
}

void MongoPushNotificationDeliveryRepository::markFailed(
    const std::string& deliveryId,
    int attempts,
    Clock::time_point attemptedAt,
    Clock::time_point nextAttemptAt,
    int responseCode,
    const std::string& error,
    bool exhausted
) {
    collection_.update_one(
        make_document(kvp("deliveryId", deliveryId)),
        make_document(kvp("$set", make_document(
            kvp("status", exhausted ? "exhausted" : "failed"),
            kvp("attempts", attempts),
            kvp("lastAttemptAt", date(attemptedAt)),
            kvp("nextAttemptAt", date(nextAttemptAt)),
            kvp("responseCode", responseCode),
            kvp("lastError", error)
        )))
    );
}

}
