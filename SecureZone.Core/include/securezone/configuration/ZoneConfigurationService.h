#pragma once

#include <string>

#include "securezone/domain/Zone.h"
#include "securezone/repository/IMachineRepository.h"
#include "securezone/repository/IZoneRepository.h"

namespace securezone::configuration {

enum class ZoneConfigurationStatus {
    Updated,
    InvalidZone,
    MachineNotFound,
    ZoneNotFound,
    RepositoryFailure
};

struct ZoneConfigurationResult {
    ZoneConfigurationStatus status{};

    bool succeeded() const {
        return status == ZoneConfigurationStatus::Updated;
    }
};

class ZoneConfigurationService {
public:
    ZoneConfigurationService(
        repository::IZoneRepository& zoneRepository,
        const repository::IMachineRepository& machineRepository
    );

    ZoneConfigurationResult create(domain::Zone zone) const;
    ZoneConfigurationResult assignXProtectEvent(
        const std::string& zoneId,
        std::string xprotectEventName
    ) const;
    ZoneConfigurationResult activate(const std::string& zoneId) const;
    ZoneConfigurationResult deactivate(const std::string& zoneId) const;
    ZoneConfigurationResult changeType(
        const std::string& zoneId,
        domain::ZoneType type
    ) const;
    ZoneConfigurationResult assignCamera(
        const std::string& zoneId,
        std::string cameraId
    ) const;
    ZoneConfigurationResult assignMachine(
        const std::string& zoneId,
        std::string machineId
    ) const;

private:
    bool machineExists(const std::string& machineId) const;
    ZoneConfigurationResult saveExisting(domain::Zone zone) const;

    repository::IZoneRepository& zoneRepository_;
    const repository::IMachineRepository& machineRepository_;
};

}
