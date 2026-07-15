#pragma once

#include <functional>
#include <optional>

#include "securezone/domain/AccessPolicy.h"
#include "securezone/domain/Employee.h"
#include "securezone/domain/MachineState.h"
#include "securezone/domain/Zone.h"
#include "securezone/domain/ZoneEntryEvent.h"

namespace securezone::decision {

struct DecisionContext {
    const domain::ZoneEntryEvent& zoneEntryEvent;
    const domain::Zone& zone;
    std::optional<std::reference_wrapper<const domain::Employee>> employee;
    const domain::MachineState& machineState;
    const domain::AccessPolicy& accessPolicy;
    bool hasZoneEntryEvent{};
    bool hadActiveAlarm{};
    bool isIdentityGracePeriodActive{};
};

}
