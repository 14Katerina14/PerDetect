#include "securezone/infrastructure/mongodb/MongoRepositoryProvider.h"

namespace securezone::infrastructure::mongodb {

namespace {

constexpr const char* AccessPoliciesCollection = "access_policies";
constexpr const char* AlarmsCollection = "alarms";
constexpr const char* CameraTracksCollection = "camera_tracks";
constexpr const char* EmployeesCollection = "employees";
constexpr const char* MachinesCollection = "machines";
constexpr const char* MetadataEventsCollection = "metadata_events";
constexpr const char* TrackIdentityBindingsCollection = "track_identity_bindings";
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

repositories::MongoCameraTrackRepository MongoRepositoryProvider::cameraTrackRepository() {
    return repositories::MongoCameraTrackRepository{
        client_.database()[CameraTracksCollection]
    };
}

repositories::MongoMetadataEventRepository MongoRepositoryProvider::metadataEventRepository() {
    return repositories::MongoMetadataEventRepository{
        client_.database()[MetadataEventsCollection]
    };
}

repositories::MongoTrackIdentityBindingRepository MongoRepositoryProvider::trackIdentityBindingRepository() {
    return repositories::MongoTrackIdentityBindingRepository{
        client_.database()[TrackIdentityBindingsCollection]
    };
}

}
