#pragma once

#include "securezone/infrastructure/mongodb/MongoDbSettings.h"

namespace securezone::infrastructure::mongodb {

MongoDbSettings loadMongoDbSettingsFromEnvironment();

}
