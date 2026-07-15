#include "securezone/infrastructure/mongodb/MongoDocumentMappers.h"

#include <bsoncxx/array/element.hpp>
#include <bsoncxx/document/element.hpp>
#include <bsoncxx/types.hpp>

#include <chrono>
#include <stdexcept>
#include <string>
#include <vector>

namespace securezone::infrastructure::mongodb {

namespace {

using Clock = std::chrono::system_clock;

template <typename StringView>
std::string toString(StringView value) {
    return std::string{value.data(), value.size()};
}

bsoncxx::document::element requiredElement(
    bsoncxx::document::view document,
    const char* fieldName
) {
    auto element = document[fieldName];
    if (!element) {
        throw std::runtime_error(std::string{"Missing MongoDB field: "} + fieldName);
    }

    return element;
}

std::string requiredString(
    bsoncxx::document::view document,
    const char* fieldName
) {
    auto element = requiredElement(document, fieldName);
    if (element.type() != bsoncxx::type::k_string) {
        throw std::runtime_error(std::string{"Expected string MongoDB field: "} + fieldName);
    }

    return toString(element.get_string().value);
}

std::string optionalString(
    bsoncxx::document::view document,
    const char* fieldName
) {
    auto element = document[fieldName];
    if (!element || element.type() == bsoncxx::type::k_null) {
        return {};
    }

    if (element.type() != bsoncxx::type::k_string) {
        throw std::runtime_error(std::string{"Expected string MongoDB field: "} + fieldName);
    }

    return toString(element.get_string().value);
}

bool optionalBool(
    bsoncxx::document::view document,
    const char* fieldName,
    bool defaultValue = false
) {
    auto element = document[fieldName];
    if (!element || element.type() == bsoncxx::type::k_null) {
        return defaultValue;
    }

    if (element.type() != bsoncxx::type::k_bool) {
        throw std::runtime_error(std::string{"Expected bool MongoDB field: "} + fieldName);
    }

    return element.get_bool().value;
}

Clock::time_point optionalDate(
    bsoncxx::document::view document,
    const char* fieldName
) {
    auto element = document[fieldName];
    if (!element || element.type() == bsoncxx::type::k_null) {
        return {};
    }

    if (element.type() != bsoncxx::type::k_date) {
        throw std::runtime_error(std::string{"Expected date MongoDB field: "} + fieldName);
    }

    return Clock::time_point{element.get_date().value};
}

Clock::time_point requiredDate(
    bsoncxx::document::view document,
    const char* fieldName
) {
    auto element = requiredElement(document, fieldName);
    if (element.type() != bsoncxx::type::k_date) {
        throw std::runtime_error(std::string{"Expected date MongoDB field: "} + fieldName);
    }

    return Clock::time_point{element.get_date().value};
}

std::vector<std::string> stringArray(
    bsoncxx::document::view document,
    const char* fieldName
) {
    auto element = requiredElement(document, fieldName);
    if (element.type() != bsoncxx::type::k_array) {
        throw std::runtime_error(std::string{"Expected array MongoDB field: "} + fieldName);
    }

    std::vector<std::string> values;
    for (const auto& item : element.get_array().value) {
        if (item.type() != bsoncxx::type::k_string) {
            throw std::runtime_error(std::string{"Expected string item in MongoDB array: "} + fieldName);
        }

        values.push_back(toString(item.get_string().value));
    }

    return values;
}

domain::EmployeeStatus employeeStatusFromString(const std::string& value) {
    if (value == "active") {
        return domain::EmployeeStatus::Active;
    }

    if (value == "inactive") {
        return domain::EmployeeStatus::Inactive;
    }

    return domain::EmployeeStatus::Inactive;
}

domain::AppUserRole appUserRoleFromString(const std::string& value) {
    if (value == "scanner") {
        return domain::AppUserRole::Scanner;
    }

    if (value == "manager") {
        return domain::AppUserRole::Manager;
    }

    if (value == "admin") {
        return domain::AppUserRole::Admin;
    }

    throw std::runtime_error("Unknown MongoDB app user role: " + value);
}

domain::AppUserStatus appUserStatusFromString(const std::string& value) {
    if (value == "active") {
        return domain::AppUserStatus::Active;
    }

    if (value == "inactive") {
        return domain::AppUserStatus::Inactive;
    }

    throw std::runtime_error("Unknown MongoDB app user status: " + value);
}

domain::ZoneType zoneTypeFromString(const std::string& value) {
    if (value == "safe") {
        return domain::ZoneType::Safe;
    }

    if (value == "restricted") {
        return domain::ZoneType::Restricted;
    }

    return domain::ZoneType::Dangerous;
}

domain::ZoneStatus zoneStatusFromString(const std::string& value) {
    if (value == "active") {
        return domain::ZoneStatus::Active;
    }

    return domain::ZoneStatus::Inactive;
}

domain::MachineStatus machineStatusFromString(const std::string& value) {
    if (value == "running") {
        return domain::MachineStatus::Running;
    }

    if (value == "maintenance") {
        return domain::MachineStatus::Maintenance;
    }

    return domain::MachineStatus::Stopped;
}

domain::AlarmStatus alarmStatusFromString(const std::string& value) {
    if (value == "created") {
        return domain::AlarmStatus::Created;
    }

    if (value == "acknowledged") {
        return domain::AlarmStatus::Acknowledged;
    }

    if (value == "resolved") {
        return domain::AlarmStatus::Resolved;
    }

    return domain::AlarmStatus::Active;
}

domain::QrCheckInStatus qrCheckinStatusFromString(const std::string& value) {
    if (value == "active") return domain::QrCheckInStatus::Active;
    if (value == "expired") return domain::QrCheckInStatus::Expired;
    return domain::QrCheckInStatus::Revoked;
}

domain::PresenceSessionStatus presenceSessionStatusFromString(const std::string& value) {
    if (value == "active") return domain::PresenceSessionStatus::Active;
    if (value == "ended") return domain::PresenceSessionStatus::Ended;
    return domain::PresenceSessionStatus::Expired;
}

domain::WebhookTargetStatus webhookTargetStatusFromString(const std::string& value) {
    return value == "active"
        ? domain::WebhookTargetStatus::Active
        : domain::WebhookTargetStatus::Inactive;
}

std::vector<domain::MachineStatus> machineStatusArray(
    bsoncxx::document::view document,
    const char* fieldName
) {
    auto values = stringArray(document, fieldName);
    std::vector<domain::MachineStatus> statuses;
    statuses.reserve(values.size());

    for (const auto& value : values) {
        statuses.push_back(machineStatusFromString(value));
    }

    return statuses;
}

}

domain::Employee mapEmployeeDocument(bsoncxx::document::view document) {
    domain::Employee employee{};
    employee.employeeId = requiredString(document, "employeeId");
    employee.fullName = requiredString(document, "fullName");
    employee.department = optionalString(document, "department");
    employee.roles = stringArray(document, "roles");
    employee.status = employeeStatusFromString(requiredString(document, "status"));
    employee.qrTokenHash = optionalString(document, "qrTokenHash");
    return employee;
}

domain::AppUser mapAppUserDocument(bsoncxx::document::view document) {
    domain::AppUser user{};
    user.userId = requiredString(document, "userId");
    user.username = requiredString(document, "username");

    if (user.userId.empty()) {
        throw std::runtime_error("MongoDB app user document has empty userId.");
    }

    if (user.username.empty()) {
        throw std::runtime_error("MongoDB app user document has empty username.");
    }

    user.role = appUserRoleFromString(requiredString(document, "role"));
    user.status = appUserStatusFromString(requiredString(document, "status"));
    return user;
}

domain::Zone mapZoneDocument(bsoncxx::document::view document) {
    domain::Zone zone{};
    zone.zoneId = requiredString(document, "zoneId");
    zone.name = requiredString(document, "name");
    zone.cameraId = requiredString(document, "cameraId");
    zone.type = zoneTypeFromString(requiredString(document, "type"));
    zone.status = zoneStatusFromString(requiredString(document, "status"));
    zone.relatedMachineId = optionalString(document, "relatedMachineId");
    zone.xprotectEventName = optionalString(document, "xprotectEventName");
    return zone;
}

domain::MachineState mapMachineDocument(bsoncxx::document::view document) {
    domain::MachineState machineState{};
    machineState.machineId = requiredString(document, "machineId");
    machineState.name = requiredString(document, "name");
    machineState.status = machineStatusFromString(requiredString(document, "status"));
    machineState.updatedAt = optionalDate(document, "updatedAt");
    return machineState;
}

domain::AccessPolicy mapAccessPolicyDocument(bsoncxx::document::view document) {
    domain::AccessPolicy accessPolicy{};
    accessPolicy.policyId = requiredString(document, "policyId");
    accessPolicy.zoneId = requiredString(document, "zoneId");
    accessPolicy.allowedRoles = stringArray(document, "allowedRoles");
    accessPolicy.machineStatesAllowed = machineStatusArray(document, "machineStatesAllowed");
    return accessPolicy;
}

domain::Alarm mapAlarmDocument(bsoncxx::document::view document) {
    domain::Alarm alarm{};
    alarm.alarmId = requiredString(document, "alarmId");
    alarm.zoneId = requiredString(document, "zoneId");
    alarm.trackId = requiredString(document, "trackId");
    alarm.employeeId = optionalString(document, "employeeId");
    alarm.machineId = optionalString(document, "machineId");
    alarm.status = alarmStatusFromString(requiredString(document, "status"));
    alarm.reason = requiredString(document, "reason");
    alarm.enteredAt = optionalDate(document, "enteredAt");
    alarm.exitedAt = optionalDate(document, "exitedAt");
    alarm.stillInside = optionalBool(document, "stillInside");
    alarm.acknowledgedBy = optionalString(document, "acknowledgedBy");
    if (auto resolvedAt = document["resolvedAt"]; resolvedAt && resolvedAt.type() != bsoncxx::type::k_null) {
        if (resolvedAt.type() != bsoncxx::type::k_date) {
            throw std::runtime_error("Expected date MongoDB field: resolvedAt");
        }
        alarm.resolvedAt = Clock::time_point{resolvedAt.get_date().value};
    }
    alarm.message = optionalString(document, "message");
    return alarm;
}

domain::QrCheckin mapQrCheckinDocument(bsoncxx::document::view document) {
    domain::QrCheckin qrCheckin{};
    qrCheckin.checkInId = requiredString(document, "checkinId");
    qrCheckin.employeeId = requiredString(document, "employeeId");
    qrCheckin.zoneId = requiredString(document, "zoneId");
    qrCheckin.scannedByUserId = requiredString(document, "scannedByUserId");
    qrCheckin.scannedAt = requiredDate(document, "scannedAt");
    qrCheckin.validUntil = requiredDate(document, "expiresAt");
    qrCheckin.status = qrCheckinStatusFromString(requiredString(document, "status"));
    return qrCheckin;
}

domain::PresenceSession mapPresenceSessionDocument(bsoncxx::document::view document) {
    domain::PresenceSession presenceSession{};
    presenceSession.sessionId = requiredString(document, "sessionId");
    presenceSession.employeeId = requiredString(document, "employeeId");
    presenceSession.zoneId = requiredString(document, "zoneId");
    presenceSession.sourceCheckinId = requiredString(document, "sourceCheckinId");
    presenceSession.startedAt = requiredDate(document, "startedAt");
    presenceSession.expiresAt = requiredDate(document, "expiresAt");
    presenceSession.endedAt = optionalDate(document, "endedAt");
    presenceSession.status = presenceSessionStatusFromString(requiredString(document, "status"));
    return presenceSession;
}

domain::WebhookTarget mapWebhookTargetDocument(bsoncxx::document::view document) {
    domain::WebhookTarget target{};
    target.targetId = requiredString(document, "targetId");
    target.name = requiredString(document, "name");
    target.url = requiredString(document, "url");
    target.status = webhookTargetStatusFromString(requiredString(document, "status"));
    target.createdAt = requiredDate(document, "createdAt");
    target.updatedAt = requiredDate(document, "updatedAt");
    return target;
}

}
