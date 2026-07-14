#include "InMemoryDecisionRepositories.h"

#include <optional>
#include <string>
#include <utility>

namespace securezone::backend {

std::optional<domain::Employee> InMemoryEmployeeRepository::findByEmployeeId(
    const std::string& employeeId
) const {
    const auto iterator = employees_.find(employeeId);
    if (iterator == employees_.end()) {
        return std::nullopt;
    }

    return iterator->second;
}

InMemoryZoneRepository::InMemoryZoneRepository(domain::Zone zone)
    : zone_{std::move(zone)} {
}

std::optional<domain::Zone> InMemoryZoneRepository::findByZoneId(
    const std::string& zoneId
) const {
    if (zone_.zoneId != zoneId) {
        return std::nullopt;
    }

    return zone_;
}

std::optional<domain::Zone> InMemoryZoneRepository::findActiveByZoneId(
    const std::string& zoneId
) const {
    if (zone_.zoneId == zoneId && zone_.status == domain::ZoneStatus::Active) {
        return zone_;
    }

    return std::nullopt;
}

InMemoryMachineRepository::InMemoryMachineRepository(domain::MachineState machine)
    : machine_{std::move(machine)} {
}

std::optional<domain::MachineState> InMemoryMachineRepository::findByMachineId(
    const std::string& machineId
) const {
    if (machine_.machineId != machineId) {
        return std::nullopt;
    }

    return machine_;
}

bool InMemoryMachineRepository::updateStatus(
    const std::string& machineId,
    domain::MachineStatus status,
    std::chrono::system_clock::time_point updatedAt
) {
    if (machine_.machineId != machineId) {
        return false;
    }

    machine_.status = status;
    machine_.updatedAt = updatedAt;
    return true;
}

InMemoryAccessPolicyRepository::InMemoryAccessPolicyRepository(
    domain::AccessPolicy accessPolicy
) : accessPolicy_{std::move(accessPolicy)} {
}

std::optional<domain::AccessPolicy> InMemoryAccessPolicyRepository::findByZoneId(
    const std::string& zoneId
) const {
    if (accessPolicy_.zoneId != zoneId) {
        return std::nullopt;
    }

    return accessPolicy_;
}

std::optional<domain::TrackIdentityBinding>
InMemoryTrackIdentityBindingRepository::findCurrentByTrackId(
    const std::string& trackId
) const {
    const auto iterator = bindings_.find(trackId);
    if (iterator == bindings_.end()) {
        return std::nullopt;
    }

    return iterator->second;
}

void InMemoryTrackIdentityBindingRepository::create(
    const domain::TrackIdentityBinding& binding
) {
    bindings_[binding.trackId] = binding;
}

void InMemoryTrackIdentityBindingRepository::updateStatus(
    const std::string& bindingId,
    const std::string& status
) {
    for (auto& [_, binding] : bindings_) {
        if (binding.bindingId != bindingId) {
            continue;
        }

        if (status == "confirmed" || status == "bound") {
            binding.status = domain::BindingStatus::Bound;
        } else if (status == "expired") {
            binding.status = domain::BindingStatus::Expired;
        } else {
            binding.status = domain::BindingStatus::Uncertain;
        }
    }
}

std::optional<domain::Alarm> InMemoryAlarmRepository::findActiveByTrackAndZone(
    const std::string& trackId,
    const std::string& zoneId
) const {
    for (const auto& alarm : alarms_) {
        if (alarm.trackId == trackId
            && alarm.zoneId == zoneId
            && alarm.status != domain::AlarmStatus::Resolved) {
            return alarm;
        }
    }

    return std::nullopt;
}

void InMemoryAlarmRepository::create(const domain::Alarm& alarm) {
    alarms_.push_back(alarm);
}

void InMemoryAlarmRepository::resolve(
    const std::string& alarmId,
    std::chrono::system_clock::time_point resolvedAt
) {
    for (auto& alarm : alarms_) {
        if (alarm.alarmId != alarmId) {
            continue;
        }

        alarm.status = domain::AlarmStatus::Resolved;
        alarm.resolvedAt = resolvedAt;
        alarm.exitedAt = resolvedAt;
        alarm.stillInside = false;
    }
}

const std::vector<domain::Alarm>& InMemoryAlarmRepository::alarms() const {
    return alarms_;
}

}
