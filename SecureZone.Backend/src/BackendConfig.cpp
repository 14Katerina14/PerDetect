#include "BackendConfig.h"

#include <fstream>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>

namespace securezone::backend {

namespace {

std::string readTextFile(const std::string& path) {
    std::ifstream file{path, std::ios::in | std::ios::binary};
    if (!file) {
        throw std::runtime_error{"Could not open backend config file: " + path};
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::string unescapeJsonString(std::string value) {
    std::string result;
    result.reserve(value.size());

    for (std::size_t index = 0; index < value.size(); ++index) {
        if (value[index] != '\\' || index + 1 >= value.size()) {
            result.push_back(value[index]);
            continue;
        }

        const char escaped = value[++index];
        switch (escaped) {
        case '"':
            result.push_back('"');
            break;
        case '\\':
            result.push_back('\\');
            break;
        case '/':
            result.push_back('/');
            break;
        case 'b':
            result.push_back('\b');
            break;
        case 'f':
            result.push_back('\f');
            break;
        case 'n':
            result.push_back('\n');
            break;
        case 'r':
            result.push_back('\r');
            break;
        case 't':
            result.push_back('\t');
            break;
        default:
            result.push_back(escaped);
            break;
        }
    }

    return result;
}

std::string findStringValue(
    const std::string& json,
    const std::string& key,
    const std::string& fallback
) {
    const std::regex pattern{
        "\"" + key + "\"\\s*:\\s*\"((?:\\\\.|[^\"])*)\""
    };

    std::smatch match;
    if (!std::regex_search(json, match, pattern)) {
        return fallback;
    }

    return unescapeJsonString(match[1].str());
}

void requireValue(const std::string& value, const std::string& key) {
    if (value.empty()) {
        throw std::runtime_error{"Missing required backend config value: " + key};
    }
}

}

BackendConfig loadBackendConfig(const std::string& path) {
    const auto json = readTextFile(path);

    BackendConfig config{};
    config.mongoConnectionStringEnv = findStringValue(
        json,
        "mongoConnectionStringEnv",
        config.mongoConnectionStringEnv
    );
    config.mongoDatabaseName = findStringValue(
        json,
        "mongoDatabaseName",
        config.mongoDatabaseName
    );
    config.cameraId = findStringValue(json, "cameraId", {});
    config.metadataInputMode = findStringValue(
        json,
        "metadataInputMode",
        config.metadataInputMode
    );
    config.metadataFilePath = findStringValue(json, "metadataFilePath", {});

    requireValue(config.mongoConnectionStringEnv, "mongoConnectionStringEnv");
    requireValue(config.mongoDatabaseName, "mongoDatabaseName");
    requireValue(config.cameraId, "cameraId");

    if (config.metadataInputMode != "file") {
        throw std::runtime_error{
            "Unsupported metadataInputMode in backend config: " + config.metadataInputMode
        };
    }

    requireValue(config.metadataFilePath, "metadataFilePath");

    return config;
}

}
