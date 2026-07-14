#include "securezone/webhook/WebhookDeliveryService.h"

#include <cassert>
#include <chrono>
#include <optional>
#include <string>
#include <vector>

namespace {

using namespace securezone;
using Clock = std::chrono::system_clock;

Clock::time_point at(int seconds) {
    return Clock::time_point{} + std::chrono::seconds{seconds};
}

webhook::AlarmNotificationPayload makePayload() {
    webhook::AlarmNotificationPayload payload{};
    payload.alarmId = "ALARM-001";
    payload.status = "active";
    payload.reason = "Employee role is not allowed in this zone.";
    payload.zoneId = "ZONE-001";
    payload.zoneName = "Dangerous Zone";
    payload.trackId = "TRACK-001";
    payload.employeeId = "EMP-001";
    payload.employeeName = "Ivan Petrov";
    payload.employeeRoles = {"maintenance", "supervisor"};
    payload.machineId = "MACHINE-001";
    payload.machineName = "Machine A";
    payload.machineStatus = "running";
    payload.enteredAt = at(10);
    payload.stillInside = true;
    payload.message = "Unauthorized presence in dangerous zone.";
    payload.timestamp = at(20);
    return payload;
}

void createsPendingDeliveryRecord() {
    const auto payload = makePayload();
    const auto createdAt = at(30);

    const auto delivery = webhook::WebhookDeliveryService{}.createPendingDelivery(
        payload,
        "https://hooks.example.test/securezone",
        createdAt
    );

    assert(!delivery.deliveryId.empty());
    assert(delivery.alarmId == "ALARM-001");
    assert(delivery.targetUrl == "https://hooks.example.test/securezone");
    assert(delivery.status == webhook::WebhookDeliveryStatus::Pending);
    assert(delivery.attempts == 0);
    assert(!delivery.lastAttemptAt.has_value());
    assert(!delivery.responseCode.has_value());
    assert(delivery.createdAt == createdAt);
}

void preservesTheEntireNotificationPayload() {
    const auto payload = makePayload();

    const auto delivery = webhook::WebhookDeliveryService{}.createPendingDelivery(
        payload,
        "https://hooks.example.test/securezone",
        at(30)
    );

    const auto& stored = delivery.payload;
    assert(stored.alarmId == payload.alarmId);
    assert(stored.status == payload.status);
    assert(stored.reason == payload.reason);
    assert(stored.zoneId == payload.zoneId);
    assert(stored.zoneName == payload.zoneName);
    assert(stored.trackId == payload.trackId);
    assert(stored.employeeId == payload.employeeId);
    assert(stored.employeeName == payload.employeeName);
    assert(stored.employeeRoles == payload.employeeRoles);
    assert(stored.machineId == payload.machineId);
    assert(stored.machineName == payload.machineName);
    assert(stored.machineStatus == payload.machineStatus);
    assert(stored.enteredAt == payload.enteredAt);
    assert(stored.stillInside == payload.stillInside);
    assert(stored.message == payload.message);
    assert(stored.timestamp == payload.timestamp);
}

}

int main() {
    createsPendingDeliveryRecord();
    preservesTheEntireNotificationPayload();
}
