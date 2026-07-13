#include "securezone/infrastructure/mongodb/MongoDbClient.h"

#include <mongocxx/uri.hpp>

namespace securezone::infrastructure::mongodb {

MongoDbClient::MongoDbClient(const MongoDbSettings& settings)
    : driverInstance_{driverInstance()},
      client_{mongocxx::uri{settings.connectionUri}},
      databaseName_{settings.databaseName} {
}

mongocxx::database MongoDbClient::database() {
    return client_[databaseName_];
}

mongocxx::instance& MongoDbClient::driverInstance() {
    static mongocxx::instance instance{};
    return instance;
}

}
