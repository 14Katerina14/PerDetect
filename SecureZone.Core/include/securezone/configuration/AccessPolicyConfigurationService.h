#pragma once

#include <string>
#include <vector>

#include "securezone/domain/AccessPolicy.h"
#include "securezone/domain/MachineState.h"
#include "securezone/repository/IAccessPolicyRepository.h"
#include "securezone/repository/IZoneRepository.h"

namespace securezone::configuration {

enum class AccessPolicyConfigurationStatus {
    Updated,
    InvalidPolicy,
    ZoneNotFound,
    PolicyNotFound,
    PolicyAlreadyExists,
    RepositoryFailure
};

struct AccessPolicyConfigurationResult {
    AccessPolicyConfigurationStatus status{};

    bool succeeded() const {
        return status == AccessPolicyConfigurationStatus::Updated;
    }
};

class AccessPolicyConfigurationService {
public:
    AccessPolicyConfigurationService(
        repository::IAccessPolicyRepository& policyRepository,
        const repository::IZoneRepository& zoneRepository
    );

    AccessPolicyConfigurationResult create(domain::AccessPolicy policy) const;
    AccessPolicyConfigurationResult updateAllowedRoles(
        const std::string& zoneId,
        std::vector<std::string> allowedRoles
    ) const;
    AccessPolicyConfigurationResult updateAllowedMachineStates(
        const std::string& zoneId,
        std::vector<domain::MachineStatus> machineStates
    ) const;
    AccessPolicyConfigurationResult replaceForZone(
        domain::AccessPolicy policy
    ) const;

private:
    AccessPolicyConfigurationResult validateAndSave(
        const domain::AccessPolicy& policy
    ) const;

    repository::IAccessPolicyRepository& policyRepository_;
    const repository::IZoneRepository& zoneRepository_;
};

}
