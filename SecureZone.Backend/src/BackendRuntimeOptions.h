#pragma once

#include <string>

namespace securezone::backend {

struct BackendRuntimeOptions {
    std::string mode{"file"};
    std::string configPath;
    bool dryRun{};
};

}
