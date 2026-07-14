#include "DemoBackendConfiguration.h"

#include <chrono>

namespace securezone::backend {

namespace {

constexpr const char* DemoZoneId = "ZONE-DEMO";
constexpr const char* DemoMachineId = "MACHINE-DEMO";

}

domain::Zone createDemoZone(const BackendConfig& config) {
    domain::Zone zone{};
    zone.zoneId = DemoZoneId;
    zone.name = "Demo Dangerous Zone";
    zone.cameraId = config.cameraId;
    zone.type = domain::ZoneType::Dangerous;
    zone.status = domain::ZoneStatus::Active;
    zone.polygon = {
        {0.0, 0.0},
        {10000.0, 0.0},
        {10000.0, 10000.0},
        {0.0, 10000.0}
    };
    zone.relatedMachineId = DemoMachineId;
    return zone;
}

domain::MachineState createDemoMachine() {
    domain::MachineState machine{};
    machine.machineId = DemoMachineId;
    machine.name = "Demo Machine";
    machine.status = domain::MachineStatus::Running;
    machine.updatedAt = std::chrono::system_clock::now();
    return machine;
}

domain::AccessPolicy createDemoAccessPolicy() {
    domain::AccessPolicy policy{};
    policy.policyId = "POLICY-DEMO";
    policy.zoneId = DemoZoneId;
    policy.allowedRoles = {"maintenance"};
    policy.machineStatesAllowed = {
        domain::MachineStatus::Stopped,
        domain::MachineStatus::Maintenance
    };
    return policy;
}

}
