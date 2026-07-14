#pragma once

#include <string>

namespace securezone::backend {

struct BackendConfig {
    std::string mongoConnectionStringEnv{"SECUREZONE_MONGO_URI"};
    std::string mongoDatabaseName{"securezone"};
    std::string cameraId;
    std::string metadataInputMode{"file"};
    std::string metadataFilePath;
};

BackendConfig loadBackendConfig(const std::string& path);

}
