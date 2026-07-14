#include "securezone/configuration/AccessPolicyConfigurationService.h"

#include <algorithm>
#include <utility>

namespace securezone::configuration {

namespace {

bool isValidMachineStatus(domain::MachineStatus status) {
    switch (status) {
        case domain::MachineStatus::Running:
        case domain::MachineStatus::Stopped:
        case domain::MachineStatus::Maintenance:
            return true;
    }

    return false;
}

bool hasInvalidMachineStatus(const std::vector<domain::MachineStatus>& statuses) {
    return std::any_of(statuses.begin(), statuses.end(), [](const auto status) {
        return !isValidMachineStatus(status);
    });
}

bool requiresRoles(domain::ZoneType type) {
    return type == domain::ZoneType::Restricted
        || type == domain::ZoneType::Dangerous;
}

bool hasEmptyRole(const std::vector<std::string>& roles) {
    return std::any_of(roles.begin(), roles.end(), [](const auto& role) {
        return role.empty();
    });
}

}

AccessPolicyConfigurationService::AccessPolicyConfigurationService(
    repository::IAccessPolicyRepository& policyRepository,
    const repository::IZoneRepository& zoneRepository
) : policyRepository_{policyRepository},
    zoneRepository_{zoneRepository} {
}

AccessPolicyConfigurationResult AccessPolicyConfigurationService::create(
    domain::AccessPolicy policy
) const {
    if (!policy.zoneId.empty() && policyRepository_.findByZoneId(policy.zoneId)) {
        return {AccessPolicyConfigurationStatus::PolicyAlreadyExists};
    }

    return validateAndSave(policy);
}

AccessPolicyConfigurationResult AccessPolicyConfigurationService::updateAllowedRoles(
    const std::string& zoneId,
    std::vector<std::string> allowedRoles
) const {
    if (zoneId.empty()) {
        return {AccessPolicyConfigurationStatus::InvalidPolicy};
    }

    auto policy = policyRepository_.findByZoneId(zoneId);
    if (!policy) {
        return {AccessPolicyConfigurationStatus::PolicyNotFound};
    }

    policy->allowedRoles = std::move(allowedRoles);
    return validateAndSave(*policy);
}

AccessPolicyConfigurationResult
AccessPolicyConfigurationService::updateAllowedMachineStates(
    const std::string& zoneId,
    std::vector<domain::MachineStatus> machineStates
) const {
    if (zoneId.empty()) {
        return {AccessPolicyConfigurationStatus::InvalidPolicy};
    }

    auto policy = policyRepository_.findByZoneId(zoneId);
    if (!policy) {
        return {AccessPolicyConfigurationStatus::PolicyNotFound};
    }

    policy->machineStatesAllowed = std::move(machineStates);
    return validateAndSave(*policy);
}

AccessPolicyConfigurationResult AccessPolicyConfigurationService::replaceForZone(
    domain::AccessPolicy policy
) const {
    return validateAndSave(policy);
}

AccessPolicyConfigurationResult AccessPolicyConfigurationService::validateAndSave(
    const domain::AccessPolicy& policy
) const {
    if (policy.zoneId.empty()
        || hasEmptyRole(policy.allowedRoles)
        || hasInvalidMachineStatus(policy.machineStatesAllowed)) {
        return {AccessPolicyConfigurationStatus::InvalidPolicy};
    }

    const auto zone = zoneRepository_.findByZoneId(policy.zoneId);
    if (!zone) {
        return {AccessPolicyConfigurationStatus::ZoneNotFound};
    }

    if (requiresRoles(zone->type) && policy.allowedRoles.empty()) {
        return {AccessPolicyConfigurationStatus::InvalidPolicy};
    }

    return {
        policyRepository_.save(policy)
            ? AccessPolicyConfigurationStatus::Updated
            : AccessPolicyConfigurationStatus::RepositoryFailure
    };
}

}
