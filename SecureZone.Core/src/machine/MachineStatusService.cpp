#include "securezone/machine/MachineStatusService.h"

#include <optional>
#include <string_view>

namespace securezone::machine {

namespace {

std::optional<domain::MachineStatus> parseMachineStatus(std::string_view value) {
    if (value == "running") {
        return domain::MachineStatus::Running;
    }

    if (value == "stopped") {
        return domain::MachineStatus::Stopped;
    }

    if (value == "maintenance") {
        return domain::MachineStatus::Maintenance;
    }

    return std::nullopt;
}

}

MachineStatusService::MachineStatusService(
    repository::IMachineRepository& machineRepository
) : machineRepository_{machineRepository} {
}

MachineStatusUpdateResult MachineStatusService::updateMachineStatus(
    const std::string& machineId,
    std::string_view newStatus,
    std::chrono::system_clock::time_point updatedAt
) const {
    const auto parsedStatus = parseMachineStatus(newStatus);
    if (machineId.empty() || !parsedStatus.has_value()) {
        return {MachineStatusUpdateStatus::InvalidStatus};
    }

    const auto machine = machineRepository_.findByMachineId(machineId);
    if (!machine.has_value()) {
        return {MachineStatusUpdateStatus::MachineNotFound};
    }

    if (machine->status == *parsedStatus) {
        return {MachineStatusUpdateStatus::Unchanged};
    }

    if (!machineRepository_.updateStatus(machineId, *parsedStatus, updatedAt)) {
        return {MachineStatusUpdateStatus::RepositoryFailure};
    }

    return {MachineStatusUpdateStatus::Updated};
}

}
