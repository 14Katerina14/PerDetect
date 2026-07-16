#pragma once

#include "securezone/domain/Zone.h"

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

namespace securezone::repository {

class IZoneStatusReadRepository {
public:
    virtual ~IZoneStatusReadRepository() = default;

    virtual std::vector<domain::Zone> findAll(
        std::size_t limit,
        const std::optional<std::string>& cameraId = std::nullopt
    ) const = 0;
};

}
