#include "securezone/configuration/ZoneConfigurationService.h"

#include <utility>

namespace securezone::configuration {

namespace {

bool isValidZoneType(domain::ZoneType type) {
    switch (type) {
        case domain::ZoneType::Safe:
        case domain::ZoneType::Restricted:
        case domain::ZoneType::Dangerous:
            return true;
    }

    return false;
}

bool isValidZone(const domain::Zone& zone) {
    return !zone.zoneId.empty()
        && !zone.name.empty()
        && !zone.cameraId.empty()
        && !zone.xprotectEventName.empty()
        && isValidZoneType(zone.type);
}

}

ZoneConfigurationService::ZoneConfigurationService(
    repository::IZoneRepository& zoneRepository,
    const repository::IMachineRepository& machineRepository
) : zoneRepository_{zoneRepository},
    machineRepository_{machineRepository} {
}

ZoneConfigurationResult ZoneConfigurationService::create(domain::Zone zone) const {
    if (!isValidZone(zone)) {
        return {ZoneConfigurationStatus::InvalidZone};
    }

    if (!machineExists(zone.relatedMachineId)) {
        return {ZoneConfigurationStatus::MachineNotFound};
    }

    return {
        zoneRepository_.save(zone)
            ? ZoneConfigurationStatus::Updated
            : ZoneConfigurationStatus::RepositoryFailure
    };
}

ZoneConfigurationResult ZoneConfigurationService::assignXProtectEvent(
    const std::string& zoneId,
    std::string xprotectEventName
) const {
    if (zoneId.empty() || xprotectEventName.empty()) {
        return {ZoneConfigurationStatus::InvalidZone};
    }

    auto zone = zoneRepository_.findByZoneId(zoneId);
    if (!zone) {
        return {ZoneConfigurationStatus::ZoneNotFound};
    }

    zone->xprotectEventName = std::move(xprotectEventName);
    return saveExisting(std::move(*zone));
}

ZoneConfigurationResult ZoneConfigurationService::activate(
    const std::string& zoneId
) const {
    if (zoneId.empty()) {
        return {ZoneConfigurationStatus::InvalidZone};
    }

    auto zone = zoneRepository_.findByZoneId(zoneId);
    if (!zone) {
        return {ZoneConfigurationStatus::ZoneNotFound};
    }

    zone->status = domain::ZoneStatus::Active;
    return saveExisting(std::move(*zone));
}

ZoneConfigurationResult ZoneConfigurationService::deactivate(
    const std::string& zoneId
) const {
    if (zoneId.empty()) {
        return {ZoneConfigurationStatus::InvalidZone};
    }

    auto zone = zoneRepository_.findByZoneId(zoneId);
    if (!zone) {
        return {ZoneConfigurationStatus::ZoneNotFound};
    }

    zone->status = domain::ZoneStatus::Inactive;
    return saveExisting(std::move(*zone));
}

ZoneConfigurationResult ZoneConfigurationService::changeType(
    const std::string& zoneId,
    domain::ZoneType type
) const {
    if (zoneId.empty() || !isValidZoneType(type)) {
        return {ZoneConfigurationStatus::InvalidZone};
    }

    auto zone = zoneRepository_.findByZoneId(zoneId);
    if (!zone) {
        return {ZoneConfigurationStatus::ZoneNotFound};
    }

    zone->type = type;
    return saveExisting(std::move(*zone));
}

ZoneConfigurationResult ZoneConfigurationService::assignCamera(
    const std::string& zoneId,
    std::string cameraId
) const {
    if (zoneId.empty() || cameraId.empty()) {
        return {ZoneConfigurationStatus::InvalidZone};
    }

    auto zone = zoneRepository_.findByZoneId(zoneId);
    if (!zone) {
        return {ZoneConfigurationStatus::ZoneNotFound};
    }

    zone->cameraId = std::move(cameraId);
    return saveExisting(std::move(*zone));
}

ZoneConfigurationResult ZoneConfigurationService::assignMachine(
    const std::string& zoneId,
    std::string machineId
) const {
    if (zoneId.empty()) {
        return {ZoneConfigurationStatus::InvalidZone};
    }

    if (!machineExists(machineId)) {
        return {ZoneConfigurationStatus::MachineNotFound};
    }

    auto zone = zoneRepository_.findByZoneId(zoneId);
    if (!zone) {
        return {ZoneConfigurationStatus::ZoneNotFound};
    }

    zone->relatedMachineId = std::move(machineId);
    return saveExisting(std::move(*zone));
}

bool ZoneConfigurationService::machineExists(const std::string& machineId) const {
    return machineId.empty() || machineRepository_.findByMachineId(machineId).has_value();
}

ZoneConfigurationResult ZoneConfigurationService::saveExisting(domain::Zone zone) const {
    if (!isValidZone(zone)) {
        return {ZoneConfigurationStatus::InvalidZone};
    }

    return {
        zoneRepository_.save(zone)
            ? ZoneConfigurationStatus::Updated
            : ZoneConfigurationStatus::RepositoryFailure
    };
}

}
