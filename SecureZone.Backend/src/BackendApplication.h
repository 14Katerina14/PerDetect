#pragma once

#include <optional>

#include "BackendConfig.h"
#include "BackendRuntimeOptions.h"

namespace securezone::backend {

class BackendApplication {
public:
    BackendApplication(
        BackendRuntimeOptions runtimeOptions,
        std::optional<BackendConfig> config
    );

    int run() const;

private:
    BackendRuntimeOptions runtimeOptions_;
    std::optional<BackendConfig> config_;
};

}
