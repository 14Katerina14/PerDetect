#include "BackendCompositionRoot.h"

#include <optional>

#include "BackendConfig.h"

namespace securezone::backend {

BackendApplication composeBackendApplication(const BackendRuntimeOptions& options) {
    std::optional<BackendConfig> config;
    if (!options.configPath.empty()) {
        config = loadBackendConfig(options.configPath);
    }

    return BackendApplication{options, config};
}

}
