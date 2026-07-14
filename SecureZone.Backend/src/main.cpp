#include <iostream>
#include <optional>
#include <string>

namespace {

struct BackendOptions {
    std::string mode{"file"};
    std::string configPath;
    bool dryRun{};
};

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

std::optional<BackendOptions> parseArguments(int argc, char** argv) {
    BackendOptions options{};

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

int run(const BackendOptions& options) {
    std::cout
        << "SecureZone Backend\n"
        << "Mode: " << options.mode << '\n';

    if (options.configPath.empty()) {
        std::cout << "Config: not provided\n";
    } else {
        std::cout << "Config: " << options.configPath << '\n';
    }

    if (options.dryRun) {
        std::cout << "Dry run: startup options are valid.\n";
        return 0;
    }

    std::cout
        << "Backend service skeleton is ready.\n"
        << "Metadata processing, MongoDB wiring, and webhook delivery records "
        << "will be added in later commits.\n";
    return 0;
}

}

int main(int argc, char** argv) {
    const auto options = parseArguments(argc, argv);
    if (!options.has_value()) {
        return argc == 1 ? 1 : 0;
    }

    return run(*options);
}
