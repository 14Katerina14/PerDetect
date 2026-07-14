#include <string>

#include <iostream>
#include <optional>

#include "BackendCompositionRoot.h"
#include "BackendRuntimeOptions.h"

namespace {

void printUsage() {
    std::cout
        << "Usage:\n"
        << "  SecureZoneBackend [--mode file] [--config securezone.backend.json] [--dry-run]\n\n"
        << "Options:\n"
        << "  --mode file       Run with local metadata file input mode.\n"
        << "  --config <path>   Path to backend configuration file.\n"
        << "  --dry-run         Validate startup options and exit.\n"
        << "  --help, -h        Show this help message.\n";
}

std::optional<securezone::backend::BackendRuntimeOptions> parseArguments(
    int argc,
    char** argv
) {
    securezone::backend::BackendRuntimeOptions options{};

    for (int index = 1; index < argc; ++index) {
        const std::string argument = argv[index];

        if (argument == "--mode" && index + 1 < argc) {
            options.mode = argv[++index];
        } else if (argument == "--config" && index + 1 < argc) {
            options.configPath = argv[++index];
        } else if (argument == "--dry-run") {
            options.dryRun = true;
        } else if (argument == "--help" || argument == "-h") {
            printUsage();
            return std::nullopt;
        } else {
            std::cerr << "Unknown or incomplete argument: " << argument << "\n\n";
            printUsage();
            return std::nullopt;
        }
    }

    if (options.mode != "file") {
        std::cerr << "Unsupported backend mode: " << options.mode << "\n\n";
        printUsage();
        return std::nullopt;
    }

    return options;
}

}

int main(int argc, char** argv) {
    const auto options = parseArguments(argc, argv);
    if (!options.has_value()) {
        return argc == 1 ? 1 : 0;
    }

    const auto application = securezone::backend::composeBackendApplication(*options);
    return application.run();
}
