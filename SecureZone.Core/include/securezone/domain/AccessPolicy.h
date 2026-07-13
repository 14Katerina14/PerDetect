#pragma once

#include <string>
#include <vector>
#include "securezone/domain/MachineState.h"
namespace securezone::domain { struct AccessPolicy { std::string policyId, zoneId; std::vector<std::string> allowedRoles; std::vector<MachineStatus> machineStatesAllowed; }; }
