#pragma once

#include "securezone/query/ManagerReadModels.h"
#include "securezone/repository/IAlarmReadRepository.h"
#include "securezone/repository/IEmployeeRepository.h"
#include "securezone/repository/IMachineRepository.h"
#include "securezone/repository/IZoneRepository.h"

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

namespace securezone::query {

class RecentAlarmQueryService {
public:
    RecentAlarmQueryService(
        repository::IAlarmReadRepository& alarmRepository,
        repository::IEmployeeRepository& employeeRepository,
        repository::IZoneRepository& zoneRepository,
        repository::IMachineRepository& machineRepository
    );

    std::vector<AlarmView> list(
        std::size_t limit = 50,
        const std::optional<std::string>& zoneId = std::nullopt
    ) const;

private:
    repository::IAlarmReadRepository& alarmRepository_;
    repository::IEmployeeRepository& employeeRepository_;
    repository::IZoneRepository& zoneRepository_;
    repository::IMachineRepository& machineRepository_;
};

}
