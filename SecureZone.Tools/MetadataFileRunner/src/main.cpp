#include <chrono>
#include <fstream>
#include <iostream>
#include <map>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#include "securezone/alarm/AlarmPersistenceService.h"
#include "securezone/decision/DecisionContextLoader.h"
#include "securezone/decision/DecisionEngine.h"
#include "securezone/domain/AccessPolicy.h"
#include "securezone/domain/Alarm.h"
#include "securezone/domain/CameraTrack.h"
#include "securezone/domain/Employee.h"
#include "securezone/domain/MachineState.h"
#include "securezone/domain/MetadataEvent.h"
#include "securezone/domain/TrackIdentityBinding.h"
#include "securezone/domain/Zone.h"
#include "securezone/geometry/ZoneGeometryService.h"
#include "securezone/metadata/MetadataApplicationService.h"
#include "securezone/metadata/MetadataDecisionTriggerService.h"
#include "securezone/metadata/MetadataIngestionService.h"
#include "securezone/metadata/MetadataPersistenceService.h"
#include "securezone/metadata/MetadataProcessingService.h"
#include "securezone/metadata/OnvifMetadataParser.h"
#include "securezone/repository/IAccessPolicyRepository.h"
#include "securezone/repository/IAlarmRepository.h"
#include "securezone/repository/ICameraTrackRepository.h"
#include "securezone/repository/IEmployeeRepository.h"
#include "securezone/repository/IMachineRepository.h"
#include "securezone/repository/IMetadataEventRepository.h"
#include "securezone/repository/ITrackIdentityBindingRepository.h"
#include "securezone/repository/IZoneRepository.h"

namespace {

using Clock = std::chrono::system_clock;
using namespace securezone;

struct RunnerOptions {
    std::string cameraId;
    std::string metadataFile;
    bool identityGracePeriodActive{};
};

class InMemoryCameraTrackRepository final : public repository::ICameraTrackRepository {
public:
    std::optional<domain::CameraTrack> findByTrackId(
        const std::string& trackId
    ) const override {
        const auto iterator = tracks_.find(trackId);
        if (iterator == tracks_.end()) {
            return std::nullopt;
        }

        return iterator->second;
    }

    void upsert(const domain::CameraTrack& cameraTrack) override {
        tracks_[cameraTrack.trackId] = cameraTrack;
    }

    void markLost(
        const std::string& trackId,
        Clock::time_point lostAt
    ) override {
        auto iterator = tracks_.find(trackId);
        if (iterator == tracks_.end()) {
            return;
        }

        iterator->second.status = "lost";
        iterator->second.lastSeenAt = lostAt;
    }

    std::size_t size() const {
        return tracks_.size();
    }

private:
    std::map<std::string, domain::CameraTrack> tracks_;
};

class InMemoryMetadataEventRepository final : public repository::IMetadataEventRepository {
public:
    void create(const domain::MetadataEvent& metadataEvent) override {
        events_.push_back(metadataEvent);
    }

    std::vector<domain::MetadataEvent> findRecentByTrackId(
        const std::string& trackId,
        std::int64_t limit
    ) const override {
        std::vector<domain::MetadataEvent> result;
        for (auto iterator = events_.rbegin(); iterator != events_.rend(); ++iterator) {
            if (iterator->trackId != trackId) {
                continue;
            }

            result.push_back(*iterator);
            if (static_cast<std::int64_t>(result.size()) >= limit) {
                break;
            }
        }

        return result;
    }

    std::size_t size() const {
        return events_.size();
    }

private:
    std::vector<domain::MetadataEvent> events_;
};

class InMemoryEmployeeRepository final : public repository::IEmployeeRepository {
public:
    std::optional<domain::Employee> findByEmployeeId(
        const std::string& employeeId
    ) const override {
        const auto iterator = employees_.find(employeeId);
        if (iterator == employees_.end()) {
            return std::nullopt;
        }

        return iterator->second;
    }

private:
    std::map<std::string, domain::Employee> employees_;
};

class InMemoryZoneRepository final : public repository::IZoneRepository {
public:
    explicit InMemoryZoneRepository(domain::Zone zone)
        : zone_{std::move(zone)} {
    }

    std::optional<domain::Zone> findByZoneId(
        const std::string& zoneId
    ) const override {
        if (zone_.zoneId != zoneId) {
            return std::nullopt;
        }

        return zone_;
    }

    std::optional<domain::Zone> findActiveByZoneId(
        const std::string& zoneId
    ) const override {
        if (zone_.zoneId == zoneId && zone_.status == domain::ZoneStatus::Active) {
            return zone_;
        }

        return std::nullopt;
    }

private:
    domain::Zone zone_;
};

class InMemoryMachineRepository final : public repository::IMachineRepository {
public:
    explicit InMemoryMachineRepository(domain::MachineState machineState)
        : machineState_{std::move(machineState)} {
    }

    std::optional<domain::MachineState> findByMachineId(
        const std::string& machineId
    ) const override {
        if (machineState_.machineId != machineId) {
            return std::nullopt;
        }

        return machineState_;
    }

    bool updateStatus(
        const std::string& machineId,
        domain::MachineStatus status,
        std::chrono::system_clock::time_point updatedAt
    ) override {
        if (machineState_.machineId != machineId) {
            return false;
        }

        machineState_.status = status;
        machineState_.updatedAt = updatedAt;
        return true;
    }

private:
    domain::MachineState machineState_;
};

class InMemoryAccessPolicyRepository final : public repository::IAccessPolicyRepository {
public:
    explicit InMemoryAccessPolicyRepository(domain::AccessPolicy accessPolicy)
        : accessPolicy_{std::move(accessPolicy)} {
    }

    std::optional<domain::AccessPolicy> findByZoneId(
        const std::string& zoneId
    ) const override {
        if (accessPolicy_.zoneId != zoneId) {
            return std::nullopt;
        }

        return accessPolicy_;
    }

private:
    domain::AccessPolicy accessPolicy_;
};

class InMemoryTrackIdentityBindingRepository final
    : public repository::ITrackIdentityBindingRepository {
public:
    std::optional<domain::TrackIdentityBinding> findCurrentByTrackId(
        const std::string& trackId
    ) const override {
        const auto iterator = bindings_.find(trackId);
        if (iterator == bindings_.end()) {
            return std::nullopt;
        }

        return iterator->second;
    }

    void create(const domain::TrackIdentityBinding& binding) override {
        bindings_[binding.trackId] = binding;
    }

    void updateStatus(
        const std::string& bindingId,
        const std::string& status
    ) override {
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

private:
    std::map<std::string, domain::TrackIdentityBinding> bindings_;
};

class InMemoryAlarmRepository final : public repository::IAlarmRepository {
public:
    std::optional<domain::Alarm> findActiveByTrackAndZone(
        const std::string& trackId,
        const std::string& zoneId
    ) const override {
        for (const auto& alarm : alarms_) {
            if (alarm.trackId == trackId
                && alarm.zoneId == zoneId
                && alarm.status != domain::AlarmStatus::Resolved) {
                return alarm;
            }
        }

        return std::nullopt;
    }

    void create(const domain::Alarm& alarm) override {
        alarms_.push_back(alarm);
    }

    void resolve(
        const std::string& alarmId,
        Clock::time_point resolvedAt
    ) override {
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

    const std::vector<domain::Alarm>& alarms() const {
        return alarms_;
    }

private:
    std::vector<domain::Alarm> alarms_;
};

void printUsage() {
    std::cout
        << "Usage:\n"
        << "  SecureZoneMetadataFileRunner --camera-id CAM-001 --metadata-file metadata.xml [--identity-grace-period]\n";
}

std::optional<RunnerOptions> parseArguments(int argc, char** argv) {
    RunnerOptions options{};

    for (int index = 1; index < argc; ++index) {
        const std::string argument = argv[index];

        if (argument == "--camera-id" && index + 1 < argc) {
            options.cameraId = argv[++index];
        } else if (argument == "--metadata-file" && index + 1 < argc) {
            options.metadataFile = argv[++index];
        } else if (argument == "--identity-grace-period") {
            options.identityGracePeriodActive = true;
        } else if (argument == "--help" || argument == "-h") {
            printUsage();
            return std::nullopt;
        } else {
            std::cerr << "Unknown or incomplete argument: " << argument << '\n';
            printUsage();
            return std::nullopt;
        }
    }

    if (options.cameraId.empty() || options.metadataFile.empty()) {
        printUsage();
        return std::nullopt;
    }

    return options;
}

std::string readTextFile(const std::string& path) {
    std::ifstream file{path, std::ios::in | std::ios::binary};
    if (!file) {
        throw std::runtime_error{"Could not open metadata file: " + path};
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

domain::Zone createDemoZone(const std::string& cameraId) {
    domain::Zone zone{};
    zone.zoneId = "ZONE-DEMO";
    zone.name = "Demo Dangerous Zone";
    zone.cameraId = cameraId;
    zone.type = domain::ZoneType::Dangerous;
    zone.status = domain::ZoneStatus::Active;
    zone.polygon = {
        {0.0, 0.0},
        {10000.0, 0.0},
        {10000.0, 10000.0},
        {0.0, 10000.0}
    };
    zone.relatedMachineId = "MACHINE-DEMO";
    return zone;
}

domain::MachineState createDemoMachine() {
    domain::MachineState machineState{};
    machineState.machineId = "MACHINE-DEMO";
    machineState.name = "Demo Machine";
    machineState.status = domain::MachineStatus::Running;
    machineState.updatedAt = Clock::now();
    return machineState;
}

domain::AccessPolicy createDemoAccessPolicy() {
    domain::AccessPolicy accessPolicy{};
    accessPolicy.policyId = "POLICY-DEMO";
    accessPolicy.zoneId = "ZONE-DEMO";
    accessPolicy.allowedRoles = {"maintenance"};
    accessPolicy.machineStatesAllowed = {
        domain::MachineStatus::Stopped,
        domain::MachineStatus::Maintenance
    };
    return accessPolicy;
}

void printSection(const std::string& title) {
    std::cout << "\n== " << title << " ==\n";
}

void printMetric(const std::string& label, std::size_t value) {
    std::cout << label << ": " << value << '\n';
}

void printResult(
    const RunnerOptions& options,
    const metadata::MetadataApplicationResult& result,
    const InMemoryAlarmRepository& alarmRepository,
    const InMemoryCameraTrackRepository& cameraTrackRepository,
    const InMemoryMetadataEventRepository& metadataEventRepository
) {
    printSection("Input");
    std::cout
        << "Camera ID: " << options.cameraId << '\n'
        << "Metadata file: " << options.metadataFile << '\n'
        << "Identity grace period: "
        << (options.identityGracePeriodActive ? "enabled" : "disabled") << '\n';

    printSection("Processing");
    printMetric("Detections processed", result.processing.detectionsProcessed);
    printMetric("Tracks upserted", result.processing.tracksUpserted);
    printMetric("Events created", result.processing.eventsCreated);

    printSection("Decision");
    printMetric("Detections checked", result.decisions.detectionsChecked);
    printMetric("Decisions evaluated", result.decisions.decisionsEvaluated);
    printMetric("Allowed", result.decisions.allowed);
    printMetric("Pending identity", result.decisions.pendingIdentity);
    printMetric("Violations", result.decisions.violations);
    printMetric("Ignored", result.decisions.ignored);

    printSection("Storage");
    printMetric("Stored tracks", cameraTrackRepository.size());
    printMetric("Stored metadata events", metadataEventRepository.size());

    printSection("Alarms");
    printMetric("Alarms created", result.decisions.alarmsCreated);
    printMetric("Alarms resolved", result.decisions.alarmsResolved);

    if (alarmRepository.alarms().empty()) {
        std::cout << "No alarms recorded.\n";
        return;
    }

    for (const auto& alarm : alarmRepository.alarms()) {
        std::cout
            << "Alarm: " << alarm.alarmId
            << " | track=" << alarm.trackId
            << " | zone=" << alarm.zoneId
            << " | reason=" << alarm.reason
            << '\n';
    }
}

int run(const RunnerOptions& options) {
    const auto rawMetadata = readTextFile(options.metadataFile);
    const auto zone = createDemoZone(options.cameraId);

    metadata::OnvifMetadataParser parser{};
    metadata::MetadataIngestionService ingestionService{parser};

    InMemoryCameraTrackRepository cameraTrackRepository{};
    InMemoryMetadataEventRepository metadataEventRepository{};
    metadata::MetadataPersistenceService persistenceService{
        cameraTrackRepository,
        metadataEventRepository
    };
    metadata::MetadataProcessingService processingService{
        ingestionService,
        persistenceService
    };

    InMemoryEmployeeRepository employeeRepository{};
    InMemoryZoneRepository zoneRepository{zone};
    InMemoryMachineRepository machineRepository{createDemoMachine()};
    InMemoryAccessPolicyRepository accessPolicyRepository{createDemoAccessPolicy()};
    decision::DecisionContextLoader contextLoader{
        employeeRepository,
        zoneRepository,
        machineRepository,
        accessPolicyRepository
    };

    geometry::ZoneGeometryService zoneGeometryService{};
    decision::DecisionEngine decisionEngine{};
    InMemoryTrackIdentityBindingRepository bindingRepository{};
    InMemoryAlarmRepository alarmRepository{};
    alarm::AlarmPersistenceService alarmPersistenceService{alarmRepository};
    metadata::MetadataDecisionTriggerService decisionTriggerService{
        zoneGeometryService,
        contextLoader,
        decisionEngine,
        alarmPersistenceService,
        bindingRepository,
        alarmRepository
    };
    metadata::MetadataApplicationService applicationService{
        processingService,
        decisionTriggerService
    };

    const auto result = applicationService.handle(
        metadata::MetadataApplicationRequest{
            options.cameraId,
            rawMetadata,
            {zone},
            options.identityGracePeriodActive
        }
    );

    printResult(options, result, alarmRepository, cameraTrackRepository, metadataEventRepository);
    return 0;
}

}

int main(int argc, char** argv) {
    try {
        const auto options = parseArguments(argc, argv);
        if (!options.has_value()) {
            return argc == 1 ? 1 : 0;
        }

        return run(*options);
    } catch (const std::exception& exception) {
        std::cerr << "Error: " << exception.what() << '\n';
        return 1;
    }
}
