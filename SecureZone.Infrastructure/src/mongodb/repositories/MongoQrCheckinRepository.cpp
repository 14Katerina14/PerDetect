#include "securezone/infrastructure/mongodb/repositories/MongoQrCheckinRepository.h"

#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>
#include <bsoncxx/types.hpp>
#include <mongocxx/options/find.hpp>

#include <chrono>
#include <utility>

#include "securezone/infrastructure/mongodb/MongoDocumentMappers.h"

namespace securezone::infrastructure::mongodb::repositories {

namespace {

using Clock = std::chrono::system_clock;

constexpr char ActiveStatus[] = "active";
constexpr char CheckinIdField[] = "checkinId";
constexpr char EmployeeIdField[] = "employeeId";
constexpr char ScannedByUserIdField[] = "scannedByUserId";
constexpr char StatusField[] = "status";
constexpr char ZoneIdField[] = "zoneId";

const char* qrCheckinStatusToString(domain::QrCheckInStatus status) {
    switch (status) {
        case domain::QrCheckInStatus::Active: return "active";
        case domain::QrCheckInStatus::Expired: return "expired";
        case domain::QrCheckInStatus::Revoked: return "revoked";
    }
    return "revoked";
}

bsoncxx::types::b_date toBsonDate(Clock::time_point timePoint) {
    return bsoncxx::types::b_date{
        std::chrono::duration_cast<std::chrono::milliseconds>(
            timePoint.time_since_epoch()
        )
    };
}

bsoncxx::document::value toQrCheckinDocument(const domain::QrCheckin& qrCheckin) {
    bsoncxx::builder::basic::document document;
    document.append(
        bsoncxx::builder::basic::kvp(CheckinIdField, qrCheckin.checkInId),
        bsoncxx::builder::basic::kvp(EmployeeIdField, qrCheckin.employeeId),
        bsoncxx::builder::basic::kvp(ZoneIdField, qrCheckin.zoneId),
        bsoncxx::builder::basic::kvp(ScannedByUserIdField, qrCheckin.scannedByUserId),
        bsoncxx::builder::basic::kvp("scannedAt", toBsonDate(qrCheckin.scannedAt)),
        bsoncxx::builder::basic::kvp("expiresAt", toBsonDate(qrCheckin.validUntil)),
        bsoncxx::builder::basic::kvp(StatusField, qrCheckinStatusToString(qrCheckin.status))
    );

    return document.extract();
}

}

MongoQrCheckinRepository::MongoQrCheckinRepository(
    mongocxx::collection qrCheckinsCollection
) : qrCheckinsCollection_{std::move(qrCheckinsCollection)} {
}

void MongoQrCheckinRepository::create(const domain::QrCheckin& qrCheckin) {
    auto document = toQrCheckinDocument(qrCheckin);
    qrCheckinsCollection_.insert_one(document.view());
}

std::optional<domain::QrCheckin> MongoQrCheckinRepository::findByCheckinId(
    const std::string& checkinId
) const {
    bsoncxx::builder::basic::document filter;
    filter.append(bsoncxx::builder::basic::kvp(CheckinIdField, checkinId));

    auto result = qrCheckinsCollection_.find_one(filter.view());
    if (!result) {
        return std::nullopt;
    }

    return mapQrCheckinDocument(result->view());
}

std::optional<domain::QrCheckin> MongoQrCheckinRepository::findLatestActiveByEmployeeAndZone(
    const std::string& employeeId,
    const std::string& zoneId
) const {
    bsoncxx::builder::basic::document filter;
    filter.append(
        bsoncxx::builder::basic::kvp(EmployeeIdField, employeeId),
        bsoncxx::builder::basic::kvp(ZoneIdField, zoneId),
        bsoncxx::builder::basic::kvp(StatusField, ActiveStatus)
    );

    bsoncxx::builder::basic::document sort;
    sort.append(bsoncxx::builder::basic::kvp("scannedAt", -1));

    mongocxx::options::find options;
    options.sort(sort.view());
    options.limit(1);

    auto cursor = qrCheckinsCollection_.find(filter.view(), options);
    auto iterator = cursor.begin();
    if (iterator == cursor.end()) {
        return std::nullopt;
    }

    return mapQrCheckinDocument(*iterator);
}

}
