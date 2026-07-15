#pragma once

#include "securezone/infrastructure/mongodb/MongoDbClient.h"
#include "securezone/infrastructure/mongodb/repositories/MongoAccessPolicyRepository.h"
#include "securezone/infrastructure/mongodb/repositories/MongoAlarmRepository.h"
#include "securezone/infrastructure/mongodb/repositories/MongoAppUserRepository.h"
#include "securezone/infrastructure/mongodb/repositories/MongoEmployeeRepository.h"
#include "securezone/infrastructure/mongodb/repositories/MongoMachineRepository.h"
#include "securezone/infrastructure/mongodb/repositories/MongoPresenceSessionRepository.h"
#include "securezone/infrastructure/mongodb/repositories/MongoQrCheckinRepository.h"
#include "securezone/infrastructure/mongodb/repositories/MongoWebhookTargetRepository.h"
#include "securezone/infrastructure/mongodb/repositories/MongoZoneRepository.h"
#include "securezone/infrastructure/mongodb/repositories/MongoCameraObjectTrackRepository.h"
#include "securezone/infrastructure/mongodb/repositories/MongoTrackIdentityBindingRepository.h"

namespace securezone::infrastructure::mongodb {

class MongoRepositoryProvider {
public:
    explicit MongoRepositoryProvider(MongoDbClient& client);

    repositories::MongoEmployeeRepository employeeRepository();
    repositories::MongoAppUserRepository appUserRepository();
    repositories::MongoZoneRepository zoneRepository();
    repositories::MongoMachineRepository machineRepository();
    repositories::MongoAccessPolicyRepository accessPolicyRepository();
    repositories::MongoAlarmRepository alarmRepository();
    repositories::MongoQrCheckinRepository qrCheckinRepository();
    repositories::MongoPresenceSessionRepository presenceSessionRepository();
    repositories::MongoWebhookTargetRepository webhookTargetRepository();
    repositories::MongoCameraObjectTrackRepository cameraObjectTrackRepository();
    repositories::MongoTrackIdentityBindingRepository trackIdentityBindingRepository();

private:
    MongoDbClient& client_;
};

}
