#include "BackendApplication.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

#include "InMemoryMetadataRepositories.h"
#include "securezone/metadata/MetadataIngestionService.h"
#include "securezone/metadata/MetadataPersistenceService.h"
#include "securezone/metadata/MetadataProcessingService.h"
#include "securezone/metadata/OnvifMetadataParser.h"

namespace securezone::backend {

namespace {

std::string readTextFile(const std::string& path) {
    std::ifstream file{path, std::ios::in | std::ios::binary};
    if (!file) {
        throw std::runtime_error{"Could not open metadata file: " + path};
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void printProcessingSummary(
    const metadata::MetadataProcessingResult& result,
    const InMemoryCameraTrackRepository& cameraTrackRepository,
    const InMemoryMetadataEventRepository& metadataEventRepository
) {
    std::cout
        << "\n== Metadata Processing ==\n"
        << "Detections processed: " << result.detectionsProcessed << '\n'
        << "Tracks upserted: " << result.tracksUpserted << '\n'
        << "Events created: " << result.eventsCreated << '\n'
        << "Stored tracks: " << cameraTrackRepository.size() << '\n'
        << "Stored metadata events: " << metadataEventRepository.size() << '\n';
}

int processMetadataFile(const BackendConfig& config) {
    const auto rawMetadata = readTextFile(config.metadataFilePath);

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

    const auto result = processingService.process(config.cameraId, rawMetadata);
    printProcessingSummary(result, cameraTrackRepository, metadataEventRepository);
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
