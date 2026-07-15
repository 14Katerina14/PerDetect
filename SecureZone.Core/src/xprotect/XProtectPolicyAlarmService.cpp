#include "securezone/xprotect/XProtectPolicyAlarmService.h"

#include <algorithm>
#include <cctype>
#include <functional>
#include <optional>
#include <string>

namespace securezone::xprotect {
namespace {

std::string normalizeAction(std::string action) {
    action.erase(
        std::remove_if(action.begin(), action.end(), [](unsigned char character) {
            return !std::isalnum(character);
        }),
        action.end()
    );
    std::transform(action.begin(), action.end(), action.begin(), [](unsigned char character) {
        return static_cast<char>(std::tolower(character));
    });
    return action;
}

bool isExitAction(const std::string& action) {
    const auto normalized = normalizeAction(action);
    return normalized == "exit"
        || normalized == "exited"
        || normalized == "leave"
        || normalized == "leaving"
        || normalized == "out"
        || normalized == "outside"
        || normalized == "outbound"
        || normalized == "stop"
        || normalized == "end";
}

std::string trackId(const XProtectLineCrossingCommand& command) {
    return command.cameraId + ":" + command.objectId;
}

XProtectLineCrossingDecision result(
    const domain::Zone& zone,
    const std::optional<domain::TrackIdentityBinding>& binding,
    std::string decision,
    std::string message
) {
    return {
        true,
        "processed",
        std::move(decision),
        zone.zoneId,
        binding.has_value() ? binding->presenceSessionId : std::string{},
        binding.has_value() ? binding->employeeId : std::string{},
        std::move(message)
    };
}

domain::AccessDecision violation(std::string reason) {
    return {
        domain::AccessDecisionType::Violation,
        std::move(reason),
        true,
        false
    };
}

}

XProtectPolicyAlarmService::XProtectPolicyAlarmService(
    repository::IEmployeeRepository& employeeRepository,
    repository::IAccessPolicyRepository& accessPolicyRepository,
    repository::IMachineRepository& machineRepository,
    repository::IAlarmRepository& alarmRepository
) : employeeRepository_{employeeRepository},
    accessPolicyRepository_{accessPolicyRepository},
    machineRepository_{machineRepository},
    alarmRepository_{alarmRepository},
    alarmPersistenceService_{alarmRepository} {
}

XProtectLineCrossingDecision XProtectPolicyAlarmService::evaluate(
    const XProtectLineCrossingCommand& command,
    const domain::Zone& zone,
    const std::optional<domain::TrackIdentityBinding>& binding
) {
    if (command.cameraId.empty() || command.objectId.empty()) {
        return result(
            zone,
            binding,
            "violation",
            "Line crossing event has no cameraId or ObjectId identity evidence."
        );
    }

    const domain::ZoneEntryEvent zoneEntryEvent{
        command.eventName,
        trackId(command),
        command.cameraId,
        command.sourceName,
        command.receivedAt
    };
    const auto activeAlarm = alarmRepository_.findActiveByTrackAndZone(
        zoneEntryEvent.trackId,
        zone.zoneId
    );

    domain::AccessPolicy policy{};
    policy.zoneId = zone.zoneId;
    domain::MachineState machine{};
    machine.machineId = zone.relatedMachineId;

    if (isExitAction(command.action)) {
        const decision::DecisionContext context{
            zoneEntryEvent,
            zone,
            std::nullopt,
            machine,
            policy,
            false,
            activeAlarm.has_value(),
            false
        };
        const auto clearDecision = decisionEngine_.evaluate(context);
        const auto action = alarmPersistenceService_.persist(
            clearDecision,
            context,
            command.receivedAt
        );

        if (action != alarm::AlarmPersistenceAction::Resolved) {
            return result(zone, binding, "allowed", "No active alarm existed for the exiting camera object.");
        }

        if (alarmRepository_.countActiveByZone(zone.zoneId) == 0U) {
            return result(zone, binding, "cleared", "The camera object exited and the zone has no active violations.");
        }

        return result(
            zone,
            binding,
            "violation_active",
            "The camera object exited, but other active violations remain in the zone."
        );
    }

    std::optional<domain::Employee> employee;
    if (binding.has_value() && binding->isActiveAt(command.receivedAt)) {
        employee = employeeRepository_.findByEmployeeId(binding->employeeId);
    }

    const auto storedPolicy = accessPolicyRepository_.findByZoneId(zone.zoneId);
    const auto storedMachine = zone.relatedMachineId.empty()
        ? std::optional<domain::MachineState>{}
        : machineRepository_.findByMachineId(zone.relatedMachineId);
    if (storedPolicy.has_value()) {
        policy = *storedPolicy;
    }
    if (storedMachine.has_value()) {
        machine = *storedMachine;
    }

    const decision::DecisionContext context{
        zoneEntryEvent,
        zone,
        employee.has_value()
            ? std::optional<std::reference_wrapper<const domain::Employee>>{std::cref(*employee)}
            : std::nullopt,
        machine,
        policy,
        true,
        activeAlarm.has_value(),
        false
    };

    domain::AccessDecision accessDecision{};
    if (binding.has_value() && !employee.has_value()) {
        accessDecision = violation("The QR identity binding references an employee that does not exist.");
    } else if (!storedPolicy.has_value()) {
        accessDecision = violation("No access policy is configured for the zone.");
    } else if (!storedMachine.has_value()) {
        accessDecision = violation("The zone machine is missing or not configured.");
    } else {
        accessDecision = decisionEngine_.evaluate(context);
    }

    const auto persistenceAction = alarmPersistenceService_.persist(
        accessDecision,
        context,
        command.receivedAt
    );

    if (accessDecision.shouldCreateAlarm) {
        return result(
            zone,
            binding,
            persistenceAction == alarm::AlarmPersistenceAction::Created
                ? "violation"
                : "violation_active",
            accessDecision.reason
        );
    }

    return result(zone, binding, "allowed", accessDecision.reason);
}

}
