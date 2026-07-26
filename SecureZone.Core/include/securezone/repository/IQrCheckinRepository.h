#pragma once

#include <optional>
#include <string>

#include "securezone/domain/QrCheckIn.h"

namespace securezone::repository {

class IQrCheckinRepository {
public:
    virtual ~IQrCheckinRepository() = default;

    virtual void create(const domain::QrCheckin& qrCheckin) = 0;

    virtual std::optional<domain::QrCheckin> findByCheckinId(
        const std::string& checkinId
    ) const = 0;

    virtual std::optional<domain::QrCheckin> findLatestActiveByEmployeeAndZone(
        const std::string& employeeId,
        const std::string& zoneId
    ) const = 0;
};

}
