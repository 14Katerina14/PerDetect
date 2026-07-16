#pragma once

#include "securezone/query/ManagerReadModels.h"
#include "securezone/repository/IMachineRepository.h"
#include "securezone/repository/IZoneStatusReadRepository.h"

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

namespace securezone::query {

class ZoneStatusQueryService {
public:
    ZoneStatusQueryService(
        repository::IZoneStatusReadRepository& zoneRepository,
        repository::IMachineRepository& machineRepository
    );

    std::vector<ZoneStatusView> list(
        std::size_t limit = 50,
        const std::optional<std::string>& cameraId = std::nullopt
    ) const;

private:
    repository::IZoneStatusReadRepository& zoneRepository_;
    repository::IMachineRepository& machineRepository_;
};

}
