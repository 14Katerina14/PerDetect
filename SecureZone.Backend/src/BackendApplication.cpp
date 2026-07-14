#include "BackendApplication.h"

#include <chrono>
#include <fstream>
#include <functional>
#include <iostream>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "DemoBackendConfiguration.h"
#include "InMemoryDecisionRepositories.h"
#include "InMemoryMetadataRepositories.h"
#include "securezone/alarm/AlarmPersistenceService.h"
#include "securezone/decision/DecisionContextLoader.h"
#include "securezone/decision/DecisionEngine.h"
#include "securezone/geometry/ZoneGeometryService.h"
#include "securezone/metadata/MetadataApplicationService.h"
#include "securezone/metadata/MetadataIngestionService.h"
#include "securezone/metadata/MetadataPersistenceService.h"
#include "securezone/metadata/MetadataProcessingService.h"
#include "securezone/metadata/OnvifMetadataParser.h"
#include "securezone/webhook/WebhookDeliveryService.h"
#include "securezone/webhook/WebhookNotificationService.h"

namespace securezone::backend {

namespace {

constexpr const char* DemoWebhookTargetUrl = "https://hooks.example.test/securezone";

std::string readTextFile(const std::string& path) {
    std::ifstream file{path, std::ios::in | std::ios::binary};
    if (!file) {
        throw std::runtime_error{"Could not open metadata file: " + path};
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::string webhookDeliveryStatusToString(webhook::WebhookDeliveryStatus status) {
    switch (status) {
    case webhook::WebhookDeliveryStatus::Pending:
        return "pending";
    case webhook::WebhookDeliveryStatus::Delivered:
        return "delivered";
    case webhook::WebhookDeliveryStatus::Failed:
        return "failed";
    }

    return "unknown";
}

std::vector<webhook::WebhookDeliveryRecord> prepareWebhookDeliveries(
    const InMemoryAlarmRepository& alarmRepository,
    const domain::Zone& zone,
    const InMemoryMachineRepository& machineRepository
) {
    webhook::WebhookNotificationService notificationService{};
    webhook::WebhookDeliveryService deliveryService{};
    std::vector<webhook::WebhookDeliveryRecord> deliveries;

    for (const auto& alarm : alarmRepository.alarms()) {
        const auto machine = alarm.machineId.empty()
            ? std::optional<domain::MachineState>{}
            : machineRepository.findByMachineId(alarm.machineId);

        const auto payload = notificationService.createAlarmNotificationPayload(
            alarm,
            webhook::AlarmNotificationContext{
                zone,
                std::nullopt,
                machine.has_value()
                    ? std::optional<std::reference_wrapper<const domain::MachineState>>{
                        std::cref(*machine)
                    }
                    : std::nullopt,
                std::chrono::system_clock::now()
            }
        );

        deliveries.push_back(
            deliveryService.createPendingDelivery(
                payload,
                DemoWebhookTargetUrl,
                std::chrono::system_clock::now()
            )
        );
    }

    return deliveries;
}

void printProcessingSummary(
    const metadata::MetadataApplicationResult& result,
    const InMemoryCameraTrackRepository& cameraTrackRepository,
    const InMemoryMetadataEventRepository& metadataEventRepository,
    const InMemoryAlarmRepository& alarmRepository,
    const std::vector<webhook::WebhookDeliveryRecord>& webhookDeliveries
) {
    std::cout
        << "\n== Metadata Processing ==\n"
        << "Detections processed: " << result.processing.detectionsProcessed << '\n'
        << "Tracks upserted: " << result.processing.tracksUpserted << '\n'
        << "Events created: " << result.processing.eventsCreated << '\n'
        << "Stored tracks: " << cameraTrackRepository.size() << '\n'
        << "Stored metadata events: " << metadataEventRepository.size() << '\n'
        << "\n== Decision Pipeline ==\n"
        << "Detections checked: " << result.decisions.detectionsChecked << '\n'
        << "Decisions evaluated: " << result.decisions.decisionsEvaluated << '\n'
        << "Allowed: " << result.decisions.allowed << '\n'
        << "Pending identity: " << result.decisions.pendingIdentity << '\n'
        << "Violations: " << result.decisions.violations << '\n'
        << "Ignored: " << result.decisions.ignored << '\n'
        << "Alarms created: " << result.decisions.alarmsCreated << '\n'
        << "Alarms resolved: " << result.decisions.alarmsResolved << '\n';

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

    std::cout
        << "\n== Webhook Delivery Records ==\n"
        << "Deliveries prepared: " << webhookDeliveries.size() << '\n'
        << "Target URL: " << DemoWebhookTargetUrl << '\n';

    if (webhookDeliveries.empty()) {
        std::cout << "No webhook delivery records prepared.\n";
        return;
    }

    for (const auto& delivery : webhookDeliveries) {
        std::cout
            << "Delivery: " << delivery.deliveryId
            << " | alarm=" << delivery.alarmId
            << " | status=" << webhookDeliveryStatusToString(delivery.status)
            << " | attempts=" << delivery.attempts
            << '\n';
    }
}

int processMetadataFile(const BackendConfig& config) {
    const auto rawMetadata = readTextFile(config.metadataFilePath);
    const auto zone = createDemoZone(config);

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
    decision::DecisionContextLoader decisionContextLoader{
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
        decisionContextLoader,
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
            config.cameraId,
            rawMetadata,
            {zone},
            false
        }
    );
    const auto webhookDeliveries = prepareWebhookDeliveries(
        alarmRepository,
        zone,
        machineRepository
    );
    printProcessingSummary(
        result,
        cameraTrackRepository,
        metadataEventRepository,
        alarmRepository,
        webhookDeliveries
    );
    return 0;
}

}

BackendApplication::BackendApplication(
    BackendRuntimeOptions runtimeOptions,
    std::optional<BackendConfig> config
) : runtimeOptions_{std::move(runtimeOptions)},
    config_{std::move(config)} {
}

int BackendApplication::run() const {
    std::cout
        << "SecureZone Backend\n"
        << "Mode: " << runtimeOptions_.mode << '\n';

    if (!config_.has_value()) {
        std::cout << "Config: not provided\n";
    } else {
        std::cout
            << "Config: " << runtimeOptions_.configPath << '\n'
            << "Config camera ID: " << config_->cameraId << '\n'
            << "Config metadata input mode: " << config_->metadataInputMode << '\n'
            << "Config metadata file: " << config_->metadataFilePath << '\n'
            << "Config Mongo database: " << config_->mongoDatabaseName << '\n'
            << "Config Mongo URI env: " << config_->mongoConnectionStringEnv << '\n';
    }

    if (runtimeOptions_.dryRun) {
        std::cout << "Dry run: startup options are valid.\n";
        return 0;
    }

    if (!config_.has_value()) {
        std::cerr << "Backend config is required when not running in dry-run mode.\n";
        return 1;
    }

    return processMetadataFile(*config_);
}

}
