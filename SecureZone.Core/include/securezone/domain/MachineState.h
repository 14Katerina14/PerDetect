#pragma once

#include <chrono>
#include <string>
namespace securezone::domain {
enum class MachineStatus { Running, Stopped, Maintenance };
struct MachineState {
    std::string machineId;
    MachineStatus status{MachineStatus::Stopped};
    std::string name;
    std::chrono::system_clock::time_point updatedAt{};
};
}
