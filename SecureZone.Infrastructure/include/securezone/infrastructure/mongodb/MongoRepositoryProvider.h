#pragma once

#include "securezone/infrastructure/mongodb/MongoDbClient.h"
#include "securezone/infrastructure/mongodb/repositories/MongoAccessPolicyRepository.h"
#include "securezone/infrastructure/mongodb/repositories/MongoAlarmRepository.h"
#include "securezone/infrastructure/mongodb/repositories/MongoEmployeeRepository.h"
#include "securezone/infrastructure/mongodb/repositories/MongoMachineRepository.h"
#include "securezone/infrastructure/mongodb/repositories/MongoZoneRepository.h"

namespace securezone::infrastructure::mongodb {

class MongoRepositoryProvider {
public:
    explicit MongoRepositoryProvider(MongoDbClient& client);

    repositories::MongoEmployeeRepository employeeRepository();
    repositories::MongoZoneRepository zoneRepository();
    repositories::MongoMachineRepository machineRepository();
    repositories::MongoAccessPolicyRepository accessPolicyRepository();
    repositories::MongoAlarmRepository alarmRepository();

private:
    MongoDbClient& client_;
};

}
