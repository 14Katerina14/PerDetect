#include "securezone/decision/DecisionContextLoader.h"

#include <functional>
#include <optional>
#include <utility>

namespace securezone::decision {

LoadedDecisionContext::LoadedDecisionContext(
    domain::ZoneEntryEvent zoneEntryEvent,
    domain::Zone zone,
    std::optional<domain::Employee> employee,
    domain::MachineState machineState,
    domain::AccessPolicy accessPolicy,
    bool hasZoneEntryEvent,
    bool hadActiveAlarm,
    bool isIdentityGracePeriodActive
) : zoneEntryEvent_{std::move(zoneEntryEvent)},
    zone_{std::move(zone)},
    employee_{std::move(employee)},
    machineState_{std::move(machineState)},
    accessPolicy_{std::move(accessPolicy)},
    hasZoneEntryEvent_{hasZoneEntryEvent},
    hadActiveAlarm_{hadActiveAlarm},
    isIdentityGracePeriodActive_{isIdentityGracePeriodActive} {
}

DecisionContext LoadedDecisionContext::toDecisionContext() const {
    std::optional<std::reference_wrapper<const domain::Employee>> employeeRef;
    if (employee_.has_value()) {
        employeeRef = std::cref(*employee_);
    }

    DecisionContext context{
        zoneEntryEvent_,
        zone_,
        employeeRef,
        machineState_,
        accessPolicy_
    };
    context.hasZoneEntryEvent = hasZoneEntryEvent_;
    context.hadActiveAlarm = hadActiveAlarm_;
    context.isIdentityGracePeriodActive = isIdentityGracePeriodActive_;
    return context;
}

DecisionContextLoader::DecisionContextLoader(
    const repository::IEmployeeRepository& employeeRepository,
    const repository::IZoneRepository& zoneRepository,
    const repository::IMachineRepository& machineRepository,
    const repository::IAccessPolicyRepository& accessPolicyRepository
) : employeeRepository_{employeeRepository},
    zoneRepository_{zoneRepository},
    machineRepository_{machineRepository},
    accessPolicyRepository_{accessPolicyRepository} {
}

std::optional<LoadedDecisionContext> DecisionContextLoader::load(
    const DecisionContextRequest& request
) const {
    auto zone = zoneRepository_.findByZoneId(request.zoneId);
    if (!zone.has_value()) {
        return std::nullopt;
    }

    if (zone->relatedMachineId.empty()) {
        return std::nullopt;
    }

    auto machineState = machineRepository_.findByMachineId(zone->relatedMachineId);
    if (!machineState.has_value()) {
        return std::nullopt;
    }

    auto accessPolicy = accessPolicyRepository_.findByZoneId(zone->zoneId);
    if (!accessPolicy.has_value()) {
        return std::nullopt;
    }

    std::optional<domain::Employee> employee;
    if (request.employeeId.has_value()) {
        employee = employeeRepository_.findByEmployeeId(*request.employeeId);
    }

    return LoadedDecisionContext{
        request.zoneEntryEvent,
        std::move(*zone),
        std::move(employee),
        std::move(*machineState),
        std::move(*accessPolicy),
        request.hasZoneEntryEvent,
        request.hadActiveAlarm,
        request.isIdentityGracePeriodActive
    };
}

}
