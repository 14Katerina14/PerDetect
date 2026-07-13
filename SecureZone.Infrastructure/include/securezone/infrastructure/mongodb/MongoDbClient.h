#pragma once

#include <mongocxx/client.hpp>
#include <mongocxx/database.hpp>
#include <mongocxx/instance.hpp>

#include <string>

#include "securezone/infrastructure/mongodb/MongoDbSettings.h"

namespace securezone::infrastructure::mongodb {

class MongoDbClient {
public:
    explicit MongoDbClient(const MongoDbSettings& settings);

    mongocxx::database database();

private:
    static mongocxx::instance& driverInstance();

    mongocxx::instance& driverInstance_;
    mongocxx::client client_;
    std::string databaseName_;
};

}
