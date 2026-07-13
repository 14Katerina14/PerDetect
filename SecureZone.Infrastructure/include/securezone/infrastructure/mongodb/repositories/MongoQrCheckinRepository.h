#pragma once

#include <mongocxx/collection.hpp>

#include <optional>
#include <string>

#include "securezone/domain/QrCheckIn.h"
#include "securezone/repository/IQrCheckinRepository.h"

namespace securezone::infrastructure::mongodb::repositories {

class MongoQrCheckinRepository final : public repository::IQrCheckinRepository {
public:
    explicit MongoQrCheckinRepository(mongocxx::collection qrCheckinsCollection);

    void create(const domain::QrCheckin& qrCheckin) override;

    std::optional<domain::QrCheckin> findByCheckinId(
        const std::string& checkinId
    ) const override;

    std::optional<domain::QrCheckin> findLatestActiveByEmployeeAndZone(
        const std::string& employeeId,
        const std::string& zoneId
    ) const override;

private:
    mongocxx::collection qrCheckinsCollection_;
};

}
