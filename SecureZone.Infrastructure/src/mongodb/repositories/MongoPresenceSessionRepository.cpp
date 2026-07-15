#include "securezone/infrastructure/mongodb/repositories/MongoPresenceSessionRepository.h"

#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>
#include <bsoncxx/types.hpp>

#include <chrono>
#include <cstddef>
#include <utility>

#include "securezone/infrastructure/mongodb/MongoDocumentMappers.h"

namespace securezone::infrastructure::mongodb::repositories {

namespace {

using Clock = std::chrono::system_clock;

constexpr char ActiveStatus[] = "active";
constexpr char EmployeeIdField[] = "employeeId";
constexpr char EndedStatus[] = "ended";
constexpr char SessionIdField[] = "sessionId";
constexpr char StatusField[] = "status";
constexpr char ZoneIdField[] = "zoneId";

const char* presenceSessionStatusToString(domain::PresenceSessionStatus status) {
    switch (status) {
        case domain::PresenceSessionStatus::Active: return ActiveStatus;
        case domain::PresenceSessionStatus::Ended: return EndedStatus;
        case domain::PresenceSessionStatus::Expired: return "expired";
    }
    return "expired";
}

bsoncxx::types::b_date toBsonDate(Clock::time_point timePoint) {
    return bsoncxx::types::b_date{
        std::chrono::duration_cast<std::chrono::milliseconds>(
            timePoint.time_since_epoch()
        )
    };
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

bsoncxx::document::value toPresenceSessionDocument(
    const domain::PresenceSession& presenceSession
) {
    bsoncxx::builder::basic::document document;
    document.append(
        bsoncxx::builder::basic::kvp(SessionIdField, presenceSession.sessionId),
        bsoncxx::builder::basic::kvp(EmployeeIdField, presenceSession.employeeId),
        bsoncxx::builder::basic::kvp(ZoneIdField, presenceSession.zoneId),
        bsoncxx::builder::basic::kvp("sourceCheckinId", presenceSession.sourceCheckinId),
        bsoncxx::builder::basic::kvp("startedAt", toBsonDate(presenceSession.startedAt)),
        bsoncxx::builder::basic::kvp("expiresAt", toBsonDate(presenceSession.expiresAt)),
        bsoncxx::builder::basic::kvp(StatusField, presenceSessionStatusToString(presenceSession.status))
    );

    appendOptionalDate(document, "endedAt", presenceSession.endedAt);

    return document.extract();
}

}

MongoPresenceSessionRepository::MongoPresenceSessionRepository(
    mongocxx::collection presenceSessionsCollection
) : presenceSessionsCollection_{std::move(presenceSessionsCollection)} {
}

std::optional<domain::PresenceSession> MongoPresenceSessionRepository::findActiveByEmployeeAndZone(
    const std::string& employeeId,
    const std::string& zoneId
) const {
    bsoncxx::builder::basic::document filter;
    filter.append(
        bsoncxx::builder::basic::kvp(EmployeeIdField, employeeId),
        bsoncxx::builder::basic::kvp(ZoneIdField, zoneId),
        bsoncxx::builder::basic::kvp(StatusField, ActiveStatus)
    );

    auto result = presenceSessionsCollection_.find_one(filter.view());
    if (!result) {
        return std::nullopt;
    }

    return mapPresenceSessionDocument(result->view());
}

void MongoPresenceSessionRepository::create(
    const domain::PresenceSession& presenceSession
) {
    auto document = toPresenceSessionDocument(presenceSession);
    presenceSessionsCollection_.insert_one(document.view());
}

void MongoPresenceSessionRepository::extend(
    const std::string& sessionId,
    Clock::time_point expiresAt
) {
    bsoncxx::builder::basic::document filter;
    filter.append(bsoncxx::builder::basic::kvp(SessionIdField, sessionId));

    bsoncxx::builder::basic::document updateFields;
    updateFields.append(
        bsoncxx::builder::basic::kvp("expiresAt", toBsonDate(expiresAt)),
        bsoncxx::builder::basic::kvp(StatusField, ActiveStatus)
    );

    bsoncxx::builder::basic::document update;
    update.append(bsoncxx::builder::basic::kvp("$set", updateFields.extract()));

    presenceSessionsCollection_.update_one(filter.view(), update.view());
}

void MongoPresenceSessionRepository::end(
    const std::string& sessionId,
    Clock::time_point endedAt
) {
    bsoncxx::builder::basic::document filter;
    filter.append(bsoncxx::builder::basic::kvp(SessionIdField, sessionId));

    bsoncxx::builder::basic::document updateFields;
    updateFields.append(
        bsoncxx::builder::basic::kvp(StatusField, EndedStatus),
        bsoncxx::builder::basic::kvp("endedAt", toBsonDate(endedAt))
    );

    bsoncxx::builder::basic::document update;
    update.append(bsoncxx::builder::basic::kvp("$set", updateFields.extract()));

    presenceSessionsCollection_.update_one(filter.view(), update.view());
}

}
