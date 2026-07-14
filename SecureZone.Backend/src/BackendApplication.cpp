#include "BackendApplication.h"

#include <iostream>
#include <utility>

namespace securezone::backend {

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

    std::cout
        << "Backend application composition is ready.\n"
        << "Metadata processing, MongoDB wiring, and webhook delivery records "
        << "will be added in later commits.\n";
    return 0;
}

}
