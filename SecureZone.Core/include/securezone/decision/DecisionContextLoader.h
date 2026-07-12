#pragma once

#include <optional>
#include <string>

#include "securezone/decision/DecisionContext.h"
#include "securezone/domain/AccessPolicy.h"
#include "securezone/domain/Detection.h"
#include "securezone/domain/Employee.h"
#include "securezone/domain/MachineState.h"
#include "securezone/domain/Zone.h"
#include "securezone/repository/IAccessPolicyRepository.h"
#include "securezone/repository/IEmployeeRepository.h"
#include "securezone/repository/IMachineRepository.h"
#include "securezone/repository/IZoneRepository.h"

namespace securezone::decision {

struct DecisionContextRequest {
    domain::Detection detection;
    std::string zoneId;
    std::optional<std::string> employeeId;
    bool isInsideZone{};
    bool hadActiveAlarm{};
    bool isIdentityGracePeriodActive{};
};

class LoadedDecisionContext {
public:
    LoadedDecisionContext(
        domain::Detection detection,
        domain::Zone zone,
        std::optional<domain::Employee> employee,
        domain::MachineState machineState,
        domain::AccessPolicy accessPolicy,
        bool isInsideZone,
        bool hadActiveAlarm,
        bool isIdentityGracePeriodActive
    );

    DecisionContext toDecisionContext() const;

private:
    domain::Detection detection_;
    domain::Zone zone_;
    std::optional<domain::Employee> employee_;
    domain::MachineState machineState_;
    domain::AccessPolicy accessPolicy_;
    bool isInsideZone_{};
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
