#pragma once

#include <chrono>
#include <string>
#include <string_view>

#include "securezone/domain/MachineState.h"
#include "securezone/repository/IMachineRepository.h"

namespace securezone::machine {

enum class MachineStatusUpdateStatus {
    Updated,
    MachineNotFound,
    InvalidStatus,
    Unchanged,
    RepositoryFailure
};

struct MachineStatusUpdateResult {
    MachineStatusUpdateStatus status{};

    bool succeeded() const {
        return status == MachineStatusUpdateStatus::Updated
            || status == MachineStatusUpdateStatus::Unchanged;
    }
};

class MachineStatusService {
public:
    explicit MachineStatusService(repository::IMachineRepository& machineRepository);

    MachineStatusUpdateResult updateMachineStatus(
        const std::string& machineId,
        std::string_view newStatus,
        std::chrono::system_clock::time_point updatedAt
    ) const;

private:
    repository::IMachineRepository& machineRepository_;
};

}
