#include "securezone/machine/MachineStatusService.h"

#include <cassert>
#include <chrono>
#include <optional>
#include <string>

namespace {

using namespace securezone;

class FakeMachineRepository final : public repository::IMachineRepository {
public:
    std::optional<domain::MachineState> machine;
    bool updateSucceeds{true};
    int updateCalls{};
    std::string updatedMachineId;
    domain::MachineStatus updatedStatus{domain::MachineStatus::Stopped};
    std::chrono::system_clock::time_point updatedAt{};

    std::optional<domain::MachineState> findByMachineId(
        const std::string& machineId
    ) const override {
        if (!machine.has_value() || machine->machineId != machineId) {
            return std::nullopt;
        }

        return machine;
    }

    bool updateStatus(
        const std::string& machineId,
        domain::MachineStatus status,
        std::chrono::system_clock::time_point value
    ) override {
        ++updateCalls;
        updatedMachineId = machineId;
        updatedStatus = status;
        updatedAt = value;
        return updateSucceeds;
    }
};

domain::MachineState makeMachine(domain::MachineStatus status) {
    domain::MachineState machine{};
    machine.machineId = "MACHINE-001";
    machine.name = "Machine A";
    machine.status = status;
    return machine;
}

void updatesAnExistingMachine() {
    FakeMachineRepository repository;
    repository.machine = makeMachine(domain::MachineStatus::Stopped);
    machine::MachineStatusService service{repository};
    const auto timestamp = std::chrono::system_clock::time_point{} + std::chrono::seconds{42};

    const auto result = service.updateMachineStatus("MACHINE-001", "running", timestamp);

    assert(result.status == machine::MachineStatusUpdateStatus::Updated);
    assert(result.succeeded());
    assert(repository.updateCalls == 1);
    assert(repository.updatedMachineId == "MACHINE-001");
    assert(repository.updatedStatus == domain::MachineStatus::Running);
    assert(repository.updatedAt == timestamp);
}

void rejectsMissingMachine() {
    FakeMachineRepository repository;
    machine::MachineStatusService service{repository};

    const auto result = service.updateMachineStatus("MISSING", "running", {});

    assert(result.status == machine::MachineStatusUpdateStatus::MachineNotFound);
    assert(!result.succeeded());
    assert(repository.updateCalls == 0);
}

void rejectsAnInvalidStatus() {
    FakeMachineRepository repository;
    repository.machine = makeMachine(domain::MachineStatus::Stopped);
    machine::MachineStatusService service{repository};

    const auto result = service.updateMachineStatus("MACHINE-001", "offline", {});

    assert(result.status == machine::MachineStatusUpdateStatus::InvalidStatus);
    assert(!result.succeeded());
    assert(repository.updateCalls == 0);
}

void doesNotWriteWhenStatusIsUnchanged() {
    FakeMachineRepository repository;
    repository.machine = makeMachine(domain::MachineStatus::Maintenance);
    machine::MachineStatusService service{repository};

    const auto result = service.updateMachineStatus("MACHINE-001", "maintenance", {});

    assert(result.status == machine::MachineStatusUpdateStatus::Unchanged);
    assert(result.succeeded());
    assert(repository.updateCalls == 0);
}

void reportsRepositoryFailure() {
    FakeMachineRepository repository;
    repository.machine = makeMachine(domain::MachineStatus::Stopped);
    repository.updateSucceeds = false;
    machine::MachineStatusService service{repository};

    const auto result = service.updateMachineStatus("MACHINE-001", "running", {});

    assert(result.status == machine::MachineStatusUpdateStatus::RepositoryFailure);
    assert(!result.succeeded());
    assert(repository.updateCalls == 1);
}

}

int main() {
    updatesAnExistingMachine();
    rejectsMissingMachine();
    rejectsAnInvalidStatus();
    doesNotWriteWhenStatusIsUnchanged();
    reportsRepositoryFailure();
}
