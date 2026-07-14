#pragma once

#include "securezone/infrastructure/mongodb/MongoDbClient.h"
#include "securezone/infrastructure/mongodb/repositories/MongoAccessPolicyRepository.h"
#include "securezone/infrastructure/mongodb/repositories/MongoAlarmRepository.h"
#include "securezone/infrastructure/mongodb/repositories/MongoCameraTrackRepository.h"
#include "securezone/infrastructure/mongodb/repositories/MongoEmployeeRepository.h"
#include "securezone/infrastructure/mongodb/repositories/MongoMachineRepository.h"
#include "securezone/infrastructure/mongodb/repositories/MongoMetadataEventRepository.h"
#include "securezone/infrastructure/mongodb/repositories/MongoPresenceSessionRepository.h"
#include "securezone/infrastructure/mongodb/repositories/MongoQrCheckinRepository.h"
#include "securezone/infrastructure/mongodb/repositories/MongoTrackIdentityBindingRepository.h"
#include "securezone/infrastructure/mongodb/repositories/MongoZoneRepository.h"
#include "securezone/infrastructure/mongodb/repositories/MongoWebhookTargetRepository.h"

namespace securezone::infrastructure::mongodb {

class MongoRepositoryProvider {
public:
    explicit MongoRepositoryProvider(MongoDbClient& client);

    repositories::MongoEmployeeRepository employeeRepository();
    repositories::MongoZoneRepository zoneRepository();
    repositories::MongoMachineRepository machineRepository();
    repositories::MongoAccessPolicyRepository accessPolicyRepository();
    repositories::MongoAlarmRepository alarmRepository();
    repositories::MongoCameraTrackRepository cameraTrackRepository();
    repositories::MongoMetadataEventRepository metadataEventRepository();
    repositories::MongoTrackIdentityBindingRepository trackIdentityBindingRepository();
    repositories::MongoQrCheckinRepository qrCheckinRepository();
    repositories::MongoPresenceSessionRepository presenceSessionRepository();
    repositories::MongoWebhookTargetRepository webhookTargetRepository();

private:
    MongoDbClient& client_;
};

}
