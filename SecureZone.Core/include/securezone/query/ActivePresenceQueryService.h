#pragma once

#include "securezone/query/ManagerReadModels.h"
#include "securezone/repository/IEmployeeRepository.h"
#include "securezone/repository/IPresenceSessionReadRepository.h"
#include "securezone/repository/IZoneRepository.h"

#include <chrono>
#include <cstddef>
#include <optional>
#include <string>
#include <vector>

namespace securezone::query {

class ActivePresenceQueryService {
public:
    ActivePresenceQueryService(
        repository::IPresenceSessionReadRepository& presenceRepository,
        repository::IEmployeeRepository& employeeRepository,
        repository::IZoneRepository& zoneRepository
    );

    std::vector<ActivePresenceView> list(
        std::chrono::system_clock::time_point at,
        std::size_t limit = 50,
        const std::optional<std::string>& zoneId = std::nullopt
    ) const;

private:
    repository::IPresenceSessionReadRepository& presenceRepository_;
    repository::IEmployeeRepository& employeeRepository_;
    repository::IZoneRepository& zoneRepository_;
};

}
