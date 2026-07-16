#include "securezone/infrastructure/mongodb/MongoDocumentMappers.h"

#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/array.hpp>
#include <bsoncxx/builder/basic/helpers.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>
#include <bsoncxx/types.hpp>

#include <cassert>
#include <chrono>
#include <stdexcept>
#include <string>

namespace {

namespace mongodb = securezone::infrastructure::mongodb;
using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::make_array;
using bsoncxx::builder::basic::make_document;

template <typename Action>
void assertThrows(Action action) {
    try {
        action();
    } catch (const std::runtime_error&) {
        return;
    }
    assert(false && "Expected malformed MongoDB document to be rejected.");
}

bsoncxx::types::b_date date(int seconds) {
    return bsoncxx::types::b_date{std::chrono::milliseconds{seconds * 1000}};
}

void rejectsUnknownZoneTypeAndStatus() {
    const auto invalidType = make_document(
        kvp("zoneId", "ZONE-001"), kvp("name", "Zone"), kvp("cameraId", "CAM-001"),
        kvp("type", "secret"), kvp("status", "active")
    );
    const auto invalidStatus = make_document(
        kvp("zoneId", "ZONE-001"), kvp("name", "Zone"), kvp("cameraId", "CAM-001"),
        kvp("type", "dangerous"), kvp("status", "deleted")
    );

    assertThrows([&] { mongodb::mapZoneDocument(invalidType.view()); });
    assertThrows([&] { mongodb::mapZoneDocument(invalidStatus.view()); });
}

void rejectsUnknownAlarmStatus() {
    const auto document = make_document(
        kvp("alarmId", "ALARM-001"), kvp("zoneId", "ZONE-001"),
        kvp("trackId", "CAM-001:42"), kvp("status", "muted"),
        kvp("reason", "Test")
    );
    assertThrows([&] { mongodb::mapAlarmDocument(document.view()); });
}

void rejectsUnknownPresenceStatus() {
    const auto document = make_document(
        kvp("sessionId", "SESSION-001"), kvp("employeeId", "EMP-001"),
        kvp("zoneId", "ZONE-001"), kvp("sourceCheckinId", "CHECKIN-001"),
        kvp("startedAt", date(10)), kvp("expiresAt", date(20)),
        kvp("status", "paused")
    );
    assertThrows([&] { mongodb::mapPresenceSessionDocument(document.view()); });
}

void rejectsUnknownMachineAndEmployeeStatuses() {
    const auto machine = make_document(
        kvp("machineId", "MACHINE-001"), kvp("name", "Machine"),
        kvp("status", "broken")
    );
    const auto employee = make_document(
        kvp("employeeId", "EMP-001"), kvp("fullName", "Ivan Petrov"),
        kvp("roles", make_array("maintenance")), kvp("status", "suspended")
    );

    assertThrows([&] { mongodb::mapMachineDocument(machine.view()); });
    assertThrows([&] { mongodb::mapEmployeeDocument(employee.view()); });
}

}

int main() {
    rejectsUnknownZoneTypeAndStatus();
    rejectsUnknownAlarmStatus();
    rejectsUnknownPresenceStatus();
    rejectsUnknownMachineAndEmployeeStatuses();
}
