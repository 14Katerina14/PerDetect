#pragma once
#include <optional>
#include <string>
#include "securezone/repository/IMachineRepository.h"
#include "securezone/repository/IZoneRepository.h"
namespace securezone::configuration {
enum class ZoneConfigurationStatus { Updated, InvalidZone, MachineNotFound, ZoneNotFound, RepositoryFailure };
struct ZoneConfigurationResult { ZoneConfigurationStatus status{}; bool succeeded() const { return status == ZoneConfigurationStatus::Updated; } };
class ZoneConfigurationService {
public:
 ZoneConfigurationService(repository::IZoneRepository& zones, const repository::IMachineRepository& machines): zones_{zones}, machines_{machines} {}
 ZoneConfigurationResult create(domain::Zone zone) const { if (!valid(zone)) return {ZoneConfigurationStatus::InvalidZone}; if (!zone.relatedMachineId.empty() && !machines_.findByMachineId(zone.relatedMachineId)) return {ZoneConfigurationStatus::MachineNotFound}; return {zones_.save(zone)?ZoneConfigurationStatus::Updated:ZoneConfigurationStatus::RepositoryFailure}; }
 ZoneConfigurationResult updatePolygon(const std::string& id, std::vector<domain::Point> polygon) const { return change(id,[&](domain::Zone& z){z.polygon=std::move(polygon); return valid(z);}); }
 ZoneConfigurationResult activate(const std::string& id) const { return change(id,[](domain::Zone& z){z.status=domain::ZoneStatus::Active; return true;}); }
 ZoneConfigurationResult deactivate(const std::string& id) const { return change(id,[](domain::Zone& z){z.status=domain::ZoneStatus::Inactive; return true;}); }
 ZoneConfigurationResult changeType(const std::string& id, domain::ZoneType type) const { return change(id,[&](domain::Zone& z){z.type=type; return true;}); }
 ZoneConfigurationResult assignCamera(const std::string& id, std::string camera) const { return change(id,[&](domain::Zone& z){z.cameraId=std::move(camera); return !z.cameraId.empty();}); }
 ZoneConfigurationResult assignMachine(const std::string& id, std::string machineId) const { if (!machineId.empty()&&!machines_.findByMachineId(machineId)) return {ZoneConfigurationStatus::MachineNotFound}; return change(id,[&](domain::Zone& z){z.relatedMachineId=std::move(machineId); return true;}); }
private:
 static bool valid(const domain::Zone& z){return !z.zoneId.empty()&&!z.name.empty()&&!z.cameraId.empty()&&z.polygon.size()>=3;}
 template<class F> ZoneConfigurationResult change(const std::string& id,F f) const { auto z=zones_.findByZoneId(id); if(!z) return {ZoneConfigurationStatus::ZoneNotFound}; if(!f(*z)) return {ZoneConfigurationStatus::InvalidZone}; return {zones_.save(*z)?ZoneConfigurationStatus::Updated:ZoneConfigurationStatus::RepositoryFailure}; }
 repository::IZoneRepository& zones_; const repository::IMachineRepository& machines_;
}; }
