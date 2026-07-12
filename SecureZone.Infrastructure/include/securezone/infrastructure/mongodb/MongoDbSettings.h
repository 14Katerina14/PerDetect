#pragma once

#include <string>

namespace securezone::infrastructure::mongodb {

struct MongoDbSettings {
    std::string connectionUri;
    std::string databaseName;
};

}
