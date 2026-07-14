#pragma once

#include <chrono>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include "securezone/domain/AccessPolicy.h"
#include "securezone/domain/Alarm.h"
#include "securezone/domain/Employee.h"
#include "securezone/domain/MachineState.h"
#include "securezone/domain/TrackIdentityBinding.h"
#include "securezone/domain/Zone.h"
#include "securezone/repository/IAccessPolicyRepository.h"
#include "securezone/repository/IAlarmRepository.h"
#include "securezone/repository/IEmployeeRepository.h"
#include "securezone/repository/IMachineRepository.h"
#include "securezone/repository/ITrackIdentityBindingRepository.h"
#include "securezone/repository/IZoneRepository.h"

namespace securezone::backend {

class InMemoryEmployeeRepository final : public repository::IEmployeeRepository {
public:
    std::optional<domain::Employee> findByEmployeeId(
        const std::string& employeeId
    ) const override;

private:
    std::map<std::string, domain::Employee> employees_;
};

class InMemoryZoneRepository final : public repository::IZoneRepository {
public:
    explicit InMemoryZoneRepository(domain::Zone zone);

    std::optional<domain::Zone> findByZoneId(
        const std::string& zoneId
    ) const override;

    std::optional<domain::Zone> findActiveByZoneId(
        const std::string& zoneId
    ) const override;

private:
    domain::Zone zone_;
};

class InMemoryMachineRepository final : public repository::IMachineRepository {
public:
    explicit InMemoryMachineRepository(domain::MachineState machine);

    std::optional<domain::MachineState> findByMachineId(
        const std::string& machineId
    ) const override;

    bool updateStatus(
        const std::string& machineId,
        domain::MachineStatus status,
        std::chrono::system_clock::time_point updatedAt
    ) override;

private:
    domain::MachineState machine_;
};

class InMemoryAccessPolicyRepository final : public repository::IAccessPolicyRepository {
public:
    explicit InMemoryAccessPolicyRepository(domain::AccessPolicy accessPolicy);

    std::optional<domain::AccessPolicy> findByZoneId(
        const std::string& zoneId
    ) const override;

private:
    domain::AccessPolicy accessPolicy_;
};

class InMemoryTrackIdentityBindingRepository final
    : public repository::ITrackIdentityBindingRepository {
public:
    std::optional<domain::TrackIdentityBinding> findCurrentByTrackId(
        const std::string& trackId
    ) const override;

    void create(const domain::TrackIdentityBinding& binding) override;

    void updateStatus(
        const std::string& bindingId,
        const std::string& status
    ) override;

private:
    std::map<std::string, domain::TrackIdentityBinding> bindings_;
};

class InMemoryAlarmRepository final : public repository::IAlarmRepository {
public:
    std::optional<domain::Alarm> findActiveByTrackAndZone(
        const std::string& trackId,
        const std::string& zoneId
    ) const override;

    void create(const domain::Alarm& alarm) override;

    void resolve(
        const std::string& alarmId,
        std::chrono::system_clock::time_point resolvedAt
    ) override;

    const std::vector<domain::Alarm>& alarms() const;

private:
    std::vector<domain::Alarm> alarms_;
};

}
