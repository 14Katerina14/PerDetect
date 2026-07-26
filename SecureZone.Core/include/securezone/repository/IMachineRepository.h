#pragma once

#include <optional>
#include <string>

#include "securezone/domain/MachineState.h"

namespace securezone::repository {

class IMachineRepository {
public:
    virtual ~IMachineRepository() = default;

    virtual std::optional<domain::MachineState> findByMachineId(
        const std::string& machineId
    ) const = 0;

    virtual bool updateStatus(
        const std::string& machineId,
        domain::MachineStatus status,
        std::chrono::system_clock::time_point updatedAt
    ) = 0;
};

}
