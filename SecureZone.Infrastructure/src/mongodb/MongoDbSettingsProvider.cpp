#include "securezone/infrastructure/mongodb/MongoDbSettingsProvider.h"

#include <cstdlib>
#include <string>

namespace securezone::infrastructure::mongodb {

namespace {

std::string environmentOrDefault(const char* name, const char* defaultValue) {
    const char* value = std::getenv(name);
    if (value == nullptr || std::string{value}.empty()) {
        return defaultValue;
    }

    return value;
}

}

MongoDbSettings loadMongoDbSettingsFromEnvironment() {
    return MongoDbSettings{
        environmentOrDefault(
            "SECUREZONE_MONGO_URI",
            "mongodb://localhost:27018/securezone"
        ),
        environmentOrDefault("SECUREZONE_MONGO_DATABASE", "securezone")
    };
}

}
