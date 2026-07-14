#include "securezone/configuration/AccessPolicyConfigurationService.h"
#include "securezone/configuration/WebhookTargetConfigurationService.h"
#include "securezone/configuration/ZoneConfigurationService.h"

#include <cassert>
#include <chrono>
#include <map>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace {

using namespace securezone;
using Clock = std::chrono::system_clock;

class FakeZoneRepository final : public repository::IZoneRepository {
public:
    std::optional<domain::Zone> findByZoneId(
        const std::string& zoneId
    ) const override {
        const auto iterator = zones.find(zoneId);
        return iterator == zones.end()
            ? std::nullopt
            : std::optional<domain::Zone>{iterator->second};
    }

    std::optional<domain::Zone> findActiveByZoneId(
        const std::string& zoneId
    ) const override {
        const auto zone = findByZoneId(zoneId);
        if (!zone || zone->status != domain::ZoneStatus::Active) {
            return std::nullopt;
        }
        return zone;
    }

    bool save(const domain::Zone& zone) override {
        if (!saveSucceeds) {
            return false;
        }
        zones[zone.zoneId] = zone;
        return true;
    }

    std::map<std::string, domain::Zone> zones;
    bool saveSucceeds{true};
};

class FakeMachineRepository final : public repository::IMachineRepository {
public:
    std::optional<domain::MachineState> findByMachineId(
        const std::string& machineId
    ) const override {
        const auto iterator = machines.find(machineId);
        return iterator == machines.end()
            ? std::nullopt
            : std::optional<domain::MachineState>{iterator->second};
    }

    bool updateStatus(
        const std::string& machineId,
        domain::MachineStatus status,
        Clock::time_point updatedAt
    ) override {
        auto iterator = machines.find(machineId);
        if (iterator == machines.end()) {
            return false;
        }
        iterator->second.status = status;
        iterator->second.updatedAt = updatedAt;
        return true;
    }

    std::map<std::string, domain::MachineState> machines;
};

class FakeAccessPolicyRepository final
    : public repository::IAccessPolicyRepository {
public:
    std::optional<domain::AccessPolicy> findByZoneId(
        const std::string& zoneId
    ) const override {
        const auto iterator = policies.find(zoneId);
        return iterator == policies.end()
            ? std::nullopt
            : std::optional<domain::AccessPolicy>{iterator->second};
    }

    bool save(const domain::AccessPolicy& policy) override {
        if (!saveSucceeds) {
            return false;
        }
        policies[policy.zoneId] = policy;
        return true;
    }

    std::map<std::string, domain::AccessPolicy> policies;
    bool saveSucceeds{true};
};

class FakeWebhookTargetRepository final
    : public repository::IWebhookTargetRepository {
public:
    std::optional<domain::WebhookTarget> findByTargetId(
        const std::string& targetId
    ) const override {
        const auto iterator = targets.find(targetId);
        return iterator == targets.end()
            ? std::nullopt
            : std::optional<domain::WebhookTarget>{iterator->second};
    }

    std::vector<domain::WebhookTarget> findActive() const override {
        std::vector<domain::WebhookTarget> active;
        for (const auto& [_, target] : targets) {
            if (target.status == domain::WebhookTargetStatus::Active) {
                active.push_back(target);
            }
        }
        return active;
    }

    bool save(const domain::WebhookTarget& target) override {
        if (!saveSucceeds) {
            return false;
        }
        targets[target.targetId] = target;
        return true;
    }

    std::map<std::string, domain::WebhookTarget> targets;
    bool saveSucceeds{true};
};

domain::Zone validZone(
    std::string zoneId = "ZONE-001",
    domain::ZoneType type = domain::ZoneType::Dangerous
) {
    domain::Zone zone{};
    zone.zoneId = std::move(zoneId);
    zone.name = "Production cell";
    zone.cameraId = "CAM-001";
    zone.type = type;
    zone.status = domain::ZoneStatus::Active;
    zone.polygon = {{0.0, 0.0}, {10.0, 0.0}, {10.0, 10.0}};
    return zone;
}

domain::AccessPolicy validPolicy(std::string zoneId = "ZONE-001") {
    domain::AccessPolicy policy{};
    policy.policyId = "POLICY-001";
    policy.zoneId = std::move(zoneId);
    policy.allowedRoles = {"maintenance"};
    policy.machineStatesAllowed = {domain::MachineStatus::Stopped};
    return policy;
}

domain::WebhookTarget validTarget(std::string targetId = "TARGET-001") {
    domain::WebhookTarget target{};
    target.targetId = std::move(targetId);
    target.name = "Alarm receiver";
    target.url = "https://example.test/securezone-webhook";
    return target;
}

void createsAndUpdatesValidZone() {
    FakeZoneRepository zones;
    FakeMachineRepository machines;
    machines.machines["MACHINE-001"] = {
        "MACHINE-001",
        domain::MachineStatus::Stopped,
        "Press",
        {}
    };
    configuration::ZoneConfigurationService service{zones, machines};

    auto zone = validZone();
    zone.relatedMachineId = "MACHINE-001";
    assert(service.create(zone).succeeded());

    assert(service.deactivate(zone.zoneId).succeeded());
    assert(zones.zones.at(zone.zoneId).status == domain::ZoneStatus::Inactive);
    assert(service.activate(zone.zoneId).succeeded());
    assert(zones.zones.at(zone.zoneId).status == domain::ZoneStatus::Active);

    const std::vector<domain::Point> polygon{
        {1.0, 1.0}, {20.0, 1.0}, {20.0, 20.0}, {1.0, 20.0}
    };
    assert(service.updatePolygon(zone.zoneId, polygon).succeeded());
    assert(zones.zones.at(zone.zoneId).polygon.size() == 4);

    assert(service.changeType(zone.zoneId, domain::ZoneType::Safe).succeeded());
    assert(zones.zones.at(zone.zoneId).type == domain::ZoneType::Safe);
    assert(service.assignCamera(zone.zoneId, "CAM-002").succeeded());
    assert(zones.zones.at(zone.zoneId).cameraId == "CAM-002");
    assert(service.assignMachine(zone.zoneId, "MACHINE-001").succeeded());
    assert(zones.zones.at(zone.zoneId).relatedMachineId == "MACHINE-001");
}

void rejectsInvalidZoneConfiguration() {
    FakeZoneRepository zones;
    FakeMachineRepository machines;
    configuration::ZoneConfigurationService service{zones, machines};

    for (const int missingField : {0, 1, 2}) {
        auto zone = validZone();
        if (missingField == 0) zone.zoneId.clear();
        if (missingField == 1) zone.name.clear();
        if (missingField == 2) zone.cameraId.clear();
        assert(
            service.create(zone).status
            == configuration::ZoneConfigurationStatus::InvalidZone
        );
    }

    auto invalidPolygon = validZone();
    invalidPolygon.polygon = {{0.0, 0.0}, {1.0, 1.0}};
    assert(
        service.create(invalidPolygon).status
        == configuration::ZoneConfigurationStatus::InvalidZone
    );

    auto invalidType = validZone();
    invalidType.type = static_cast<domain::ZoneType>(99);
    assert(
        service.create(invalidType).status
        == configuration::ZoneConfigurationStatus::InvalidZone
    );

    auto missingMachine = validZone();
    missingMachine.relatedMachineId = "MISSING";
    assert(
        service.create(missingMachine).status
        == configuration::ZoneConfigurationStatus::MachineNotFound
    );

    zones.zones["ZONE-001"] = validZone();
    assert(
        service.assignMachine("ZONE-001", "MISSING").status
        == configuration::ZoneConfigurationStatus::MachineNotFound
    );
    assert(
        service.updatePolygon("ZONE-001", {{0.0, 0.0}, {1.0, 1.0}}).status
        == configuration::ZoneConfigurationStatus::InvalidZone
    );
}

void createsAndUpdatesAccessPolicy() {
    FakeZoneRepository zones;
    zones.zones["ZONE-001"] = validZone();
    FakeAccessPolicyRepository policies;
    configuration::AccessPolicyConfigurationService service{policies, zones};

    assert(service.create(validPolicy()).succeeded());
    assert(
        service.updateAllowedRoles("ZONE-001", {"operator", "supervisor"})
            .succeeded()
    );
    assert(policies.policies.at("ZONE-001").allowedRoles.size() == 2);

    assert(
        service.updateAllowedMachineStates(
            "ZONE-001",
            {domain::MachineStatus::Stopped, domain::MachineStatus::Maintenance}
        ).succeeded()
    );
    assert(policies.policies.at("ZONE-001").machineStatesAllowed.size() == 2);

    auto replacement = validPolicy();
    replacement.policyId = "POLICY-REPLACEMENT";
    replacement.allowedRoles = {"safety-officer"};
    assert(service.replaceForZone(replacement).succeeded());
    assert(policies.policies.at("ZONE-001").policyId == "POLICY-REPLACEMENT");
}

void rejectsInvalidAccessPolicyConfiguration() {
    FakeZoneRepository zones;
    FakeAccessPolicyRepository policies;
    configuration::AccessPolicyConfigurationService service{policies, zones};

    assert(
        service.create(validPolicy("MISSING")).status
        == configuration::AccessPolicyConfigurationStatus::ZoneNotFound
    );

    zones.zones["ZONE-001"] = validZone();
    auto noRoles = validPolicy();
    noRoles.allowedRoles.clear();
    assert(
        service.create(noRoles).status
        == configuration::AccessPolicyConfigurationStatus::InvalidPolicy
    );

    zones.zones["ZONE-002"] = validZone(
        "ZONE-002",
        domain::ZoneType::Restricted
    );
    auto restrictedNoRoles = validPolicy("ZONE-002");
    restrictedNoRoles.allowedRoles.clear();
    assert(
        service.create(restrictedNoRoles).status
        == configuration::AccessPolicyConfigurationStatus::InvalidPolicy
    );

    auto invalidState = validPolicy();
    invalidState.machineStatesAllowed = {static_cast<domain::MachineStatus>(99)};
    assert(
        service.create(invalidState).status
        == configuration::AccessPolicyConfigurationStatus::InvalidPolicy
    );
}

void configuresWebhookTargets() {
    FakeWebhookTargetRepository targets;
    configuration::WebhookTargetConfigurationService service{targets};
    const auto createdAt = Clock::time_point{std::chrono::seconds{100}};
    const auto updatedAt = Clock::time_point{std::chrono::seconds{200}};

    assert(service.add(validTarget(), createdAt).succeeded());
    const auto& created = targets.targets.at("TARGET-001");
    assert(created.status == domain::WebhookTargetStatus::Active);
    assert(created.createdAt == createdAt);
    assert(created.updatedAt == createdAt);

    assert(service.disable("TARGET-001", updatedAt).succeeded());
    assert(
        targets.targets.at("TARGET-001").status
        == domain::WebhookTargetStatus::Inactive
    );
    assert(targets.targets.at("TARGET-001").updatedAt == updatedAt);
    assert(service.listActive().empty());

    assert(service.enable("TARGET-001", updatedAt).succeeded());
    assert(service.listActive().size() == 1);
}

void rejectsInvalidWebhookTargetConfiguration() {
    FakeWebhookTargetRepository targets;
    configuration::WebhookTargetConfigurationService service{targets};

    for (const int missingField : {0, 1, 2}) {
        auto target = validTarget();
        if (missingField == 0) target.targetId.clear();
        if (missingField == 1) target.name.clear();
        if (missingField == 2) target.url.clear();
        assert(
            service.add(target).status
            == configuration::WebhookTargetConfigurationStatus::InvalidTarget
        );
    }

    auto insecure = validTarget();
    insecure.url = "http://example.test/webhook";
    assert(
        service.add(insecure).status
        == configuration::WebhookTargetConfigurationStatus::InvalidTarget
    );

    assert(
        service.enable("MISSING").status
        == configuration::WebhookTargetConfigurationStatus::TargetNotFound
    );
    assert(
        service.disable("MISSING").status
        == configuration::WebhookTargetConfigurationStatus::TargetNotFound
    );
}

}

int main() {
    createsAndUpdatesValidZone();
    rejectsInvalidZoneConfiguration();
    createsAndUpdatesAccessPolicy();
    rejectsInvalidAccessPolicyConfiguration();
    configuresWebhookTargets();
    rejectsInvalidWebhookTargetConfiguration();
}
