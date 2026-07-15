#include "securezone/infrastructure/mongodb/MongoRepositoryProvider.h"

namespace securezone::infrastructure::mongodb {

namespace {

constexpr const char* AccessPoliciesCollection = "access_policies";
constexpr const char* AlarmsCollection = "alarms";
constexpr const char* EmployeesCollection = "employees";
constexpr const char* MachinesCollection = "machines";
constexpr const char* PresenceSessionsCollection = "presence_sessions";
constexpr const char* QrCheckinsCollection = "qr_checkins";
constexpr const char* WebhookTargetsCollection = "webhook_targets";
constexpr const char* ZonesCollection = "zones";

}

MongoRepositoryProvider::MongoRepositoryProvider(MongoDbClient& client)
    : client_{client} {
}

repositories::MongoEmployeeRepository MongoRepositoryProvider::employeeRepository() {
    return repositories::MongoEmployeeRepository{
        client_.database()[EmployeesCollection]
    };
}

repositories::MongoZoneRepository MongoRepositoryProvider::zoneRepository() {
    return repositories::MongoZoneRepository{
        client_.database()[ZonesCollection]
    };
}

repositories::MongoMachineRepository MongoRepositoryProvider::machineRepository() {
    return repositories::MongoMachineRepository{
        client_.database()[MachinesCollection]
    };
}

repositories::MongoAccessPolicyRepository MongoRepositoryProvider::accessPolicyRepository() {
    return repositories::MongoAccessPolicyRepository{
        client_.database()[AccessPoliciesCollection]
    };
}

repositories::MongoAlarmRepository MongoRepositoryProvider::alarmRepository() {
    return repositories::MongoAlarmRepository{
        client_.database()[AlarmsCollection]
    };
}

repositories::MongoQrCheckinRepository MongoRepositoryProvider::qrCheckinRepository() {
    return repositories::MongoQrCheckinRepository{
        client_.database()[QrCheckinsCollection]
    };
}

repositories::MongoPresenceSessionRepository MongoRepositoryProvider::presenceSessionRepository() {
    return repositories::MongoPresenceSessionRepository{
        client_.database()[PresenceSessionsCollection]
    };
}

repositories::MongoWebhookTargetRepository MongoRepositoryProvider::webhookTargetRepository() {
    return repositories::MongoWebhookTargetRepository{
        client_.database()[WebhookTargetsCollection]
    };
}

}
