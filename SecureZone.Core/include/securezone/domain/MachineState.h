#pragma once

#include <string>
namespace securezone::domain {
enum class MachineStatus { Running, Stopped, Maintenance };
struct MachineState { std::string machineId; MachineStatus status{MachineStatus::Stopped}; };
}
