#pragma once

#include <optional>
#include <string>

#include "securezone/domain/Zone.h"

namespace securezone::repository {

class IZoneRepository {
public:
    virtual ~IZoneRepository() = default;

    virtual std::optional<domain::Zone> findByZoneId(
        const std::string& zoneId
    ) const = 0;

    virtual std::optional<domain::Zone> findActiveByZoneId(
        const std::string& zoneId
    ) const = 0;

    virtual std::optional<domain::Zone> findActiveByXProtectEventName(
        const std::string& xprotectEventName
    ) const {
        (void)xprotectEventName;
        return std::nullopt;
    }

    virtual std::optional<domain::Zone> findActiveSafeByCameraId(
        const std::string& cameraId
    ) const {
        (void)cameraId;
        return std::nullopt;
    }

    virtual bool save(const domain::Zone& zone) = 0;
};

}
