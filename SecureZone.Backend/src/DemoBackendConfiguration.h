#pragma once

#include "BackendConfig.h"
#include "securezone/domain/AccessPolicy.h"
#include "securezone/domain/MachineState.h"
#include "securezone/domain/Zone.h"

namespace securezone::backend {

domain::Zone createDemoZone(const BackendConfig& config);
domain::MachineState createDemoMachine();
domain::AccessPolicy createDemoAccessPolicy();

}
