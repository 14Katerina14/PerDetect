#include "securezone/infrastructure/mongodb/repositories/MongoAlarmRepository.h"

#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>
#include <bsoncxx/types.hpp>

#include <chrono>
#include <cstddef>
#include <optional>
#include <utility>

#include "securezone/infrastructure/mongodb/MongoDocumentMappers.h"

namespace securezone::infrastructure::mongodb::repositories {

namespace {

using Clock = std::chrono::system_clock;

constexpr char ActiveStatus[] = "active";
constexpr char AlarmIdField[] = "alarmId";
constexpr char CreatedStatus[] = "created";
constexpr char ResolvedStatus[] = "resolved";
constexpr char StatusField[] = "status";
constexpr char TrackIdField[] = "trackId";
constexpr char ZoneIdField[] = "zoneId";

std::string alarmStatusToString(domain::AlarmStatus status) {
    switch (status) {
        case domain::AlarmStatus::Created:
            return CreatedStatus;
        case domain::AlarmStatus::Acknowledged:
            return "acknowledged";
        case domain::AlarmStatus::Resolved:
            return ResolvedStatus;
        case domain::AlarmStatus::Active:
        default:
            return ActiveStatus;
    }
}

bsoncxx::types::b_date toBsonDate(Clock::time_point timePoint) {
    return bsoncxx::types::b_date{
        std::chrono::duration_cast<std::chrono::milliseconds>(
            timePoint.time_since_epoch()
        )
    };
}

template <std::size_t FieldNameSize>
void appendOptionalString(
    bsoncxx::builder::basic::document& document,
    const char (&fieldName)[FieldNameSize],
    const std::string& value
) {
    if (value.empty()) {
        document.append(bsoncxx::builder::basic::kvp(fieldName, bsoncxx::types::b_null{}));
        return;
    }

    document.append(bsoncxx::builder::basic::kvp(fieldName, value));
}

template <std::size_t FieldNameSize>
void appendOptionalDate(
    bsoncxx::builder::basic::document& document,
    const char (&fieldName)[FieldNameSize],
    Clock::time_point value
) {
    if (value == Clock::time_point{}) {
        document.append(bsoncxx::builder::basic::kvp(fieldName, bsoncxx::types::b_null{}));
        return;
    }

    document.append(bsoncxx::builder::basic::kvp(fieldName, toBsonDate(value)));
}

template <std::size_t FieldNameSize>
void appendOptionalDate(
    bsoncxx::builder::basic::document& document,
    const char (&fieldName)[FieldNameSize],
    const std::optional<Clock::time_point>& value
) {
    if (!value) {
        document.append(bsoncxx::builder::basic::kvp(fieldName, bsoncxx::types::b_null{}));
        return;
    }

    document.append(bsoncxx::builder::basic::kvp(fieldName, toBsonDate(*value)));
}

bsoncxx::document::value toAlarmDocument(const domain::Alarm& alarm) {
    bsoncxx::builder::basic::document document;

    document.append(
        bsoncxx::builder::basic::kvp(AlarmIdField, alarm.alarmId),
        bsoncxx::builder::basic::kvp(ZoneIdField, alarm.zoneId),
        bsoncxx::builder::basic::kvp(TrackIdField, alarm.trackId),
        bsoncxx::builder::basic::kvp(StatusField, alarmStatusToString(alarm.status)),
        bsoncxx::builder::basic::kvp("reason", alarm.reason),
        bsoncxx::builder::basic::kvp("stillInside", alarm.stillInside)
    );

    appendOptionalString(document, "employeeId", alarm.employeeId);
    appendOptionalString(document, "machineId", alarm.machineId);
    appendOptionalDate(document, "enteredAt", alarm.enteredAt);
    appendOptionalDate(document, "exitedAt", alarm.exitedAt);
    appendOptionalString(document, "acknowledgedBy", alarm.acknowledgedBy);
    appendOptionalDate(document, "resolvedAt", alarm.resolvedAt);
    appendOptionalString(document, "message", alarm.message);

    return document.extract();
}

}

MongoAlarmRepository::MongoAlarmRepository(
    mongocxx::collection alarmsCollection
) : alarmsCollection_{std::move(alarmsCollection)} {
}

std::optional<domain::Alarm> MongoAlarmRepository::findActiveByTrackAndZone(
    const std::string& trackId,
    const std::string& zoneId
) const {
    bsoncxx::builder::basic::document filter;
    filter.append(
        bsoncxx::builder::basic::kvp(TrackIdField, trackId),
        bsoncxx::builder::basic::kvp(ZoneIdField, zoneId),
        bsoncxx::builder::basic::kvp(StatusField, ActiveStatus)
    );

    auto result = alarmsCollection_.find_one(filter.view());
    if (!result) {
        return std::nullopt;
    }

    return mapAlarmDocument(result->view());
}

void MongoAlarmRepository::create(const domain::Alarm& alarm) {
    auto document = toAlarmDocument(alarm);
    alarmsCollection_.insert_one(document.view());
}

void MongoAlarmRepository::resolve(
    const std::string& alarmId,
    Clock::time_point resolvedAt
) {
    bsoncxx::builder::basic::document filter;
    filter.append(bsoncxx::builder::basic::kvp(AlarmIdField, alarmId));

    bsoncxx::builder::basic::document updateFields;
    updateFields.append(
        bsoncxx::builder::basic::kvp(StatusField, ResolvedStatus),
        bsoncxx::builder::basic::kvp("resolvedAt", toBsonDate(resolvedAt)),
        bsoncxx::builder::basic::kvp("exitedAt", toBsonDate(resolvedAt)),
        bsoncxx::builder::basic::kvp("stillInside", false)
    );

    bsoncxx::builder::basic::document update;
    update.append(bsoncxx::builder::basic::kvp("$set", updateFields.extract()));

    alarmsCollection_.update_one(filter.view(), update.view());
}

}
