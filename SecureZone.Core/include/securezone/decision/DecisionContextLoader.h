#pragma once

#include <optional>
#include <string>

#include "securezone/decision/DecisionContext.h"
#include "securezone/domain/AccessPolicy.h"
#include "securezone/domain/Employee.h"
#include "securezone/domain/MachineState.h"
#include "securezone/domain/Zone.h"
#include "securezone/domain/ZoneEntryEvent.h"
#include "securezone/repository/IAccessPolicyRepository.h"
#include "securezone/repository/IEmployeeRepository.h"
#include "securezone/repository/IMachineRepository.h"
#include "securezone/repository/IZoneRepository.h"

namespace securezone::decision {

struct DecisionContextRequest {
    domain::ZoneEntryEvent zoneEntryEvent;
    std::string zoneId;
    std::optional<std::string> employeeId;
    bool hasZoneEntryEvent{};
    bool hadActiveAlarm{};
    bool isIdentityGracePeriodActive{};
};

class LoadedDecisionContext {
public:
    LoadedDecisionContext(
        domain::ZoneEntryEvent zoneEntryEvent,
        domain::Zone zone,
        std::optional<domain::Employee> employee,
        domain::MachineState machineState,
        domain::AccessPolicy accessPolicy,
        bool hasZoneEntryEvent,
        bool hadActiveAlarm,
        bool isIdentityGracePeriodActive
    );

    DecisionContext toDecisionContext() const;

private:
    domain::ZoneEntryEvent zoneEntryEvent_;
    domain::Zone zone_;
    std::optional<domain::Employee> employee_;
    domain::MachineState machineState_;
    domain::AccessPolicy accessPolicy_;
    bool hasZoneEntryEvent_{};
    bool hadActiveAlarm_{};
    bool isIdentityGracePeriodActive_{};
};

class DecisionContextLoader {
public:
    DecisionContextLoader(
        const repository::IEmployeeRepository& employeeRepository,
        const repository::IZoneRepository& zoneRepository,
        const repository::IMachineRepository& machineRepository,
        const repository::IAccessPolicyRepository& accessPolicyRepository
    );

    std::optional<LoadedDecisionContext> load(
        const DecisionContextRequest& request
    ) const;

private:
    const repository::IEmployeeRepository& employeeRepository_;
    const repository::IZoneRepository& zoneRepository_;
    const repository::IMachineRepository& machineRepository_;
    const repository::IAccessPolicyRepository& accessPolicyRepository_;
};

}
