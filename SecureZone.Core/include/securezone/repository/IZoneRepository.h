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
};

}
