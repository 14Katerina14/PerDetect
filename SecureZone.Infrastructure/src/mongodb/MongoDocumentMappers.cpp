#include "securezone/infrastructure/mongodb/MongoDocumentMappers.h"

#include <bsoncxx/array/element.hpp>
#include <bsoncxx/document/element.hpp>
#include <bsoncxx/types.hpp>

#include <chrono>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace securezone::infrastructure::mongodb {

namespace {

using Clock = std::chrono::system_clock;

std::string toString(std::string_view value) {
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
    if (element.type() != bsoncxx::type::k_utf8) {
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

    if (element.type() != bsoncxx::type::k_utf8) {
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

double requiredNumber(
    bsoncxx::document::view document,
    const char* fieldName
) {
    auto element = requiredElement(document, fieldName);

    switch (element.type()) {
        case bsoncxx::type::k_int32:
            return element.get_int32().value;
        case bsoncxx::type::k_int64:
            return static_cast<double>(element.get_int64().value);
        case bsoncxx::type::k_double:
            return element.get_double().value;
        default:
            throw std::runtime_error(std::string{"Expected numeric MongoDB field: "} + fieldName);
    }
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
        if (item.type() != bsoncxx::type::k_utf8) {
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

std::vector<domain::Point> polygonFromDocument(
    bsoncxx::document::view document,
    const char* fieldName
) {
    auto element = requiredElement(document, fieldName);
    if (element.type() != bsoncxx::type::k_array) {
        throw std::runtime_error(std::string{"Expected array MongoDB field: "} + fieldName);
    }

    std::vector<domain::Point> polygon;
    for (const auto& item : element.get_array().value) {
        if (item.type() != bsoncxx::type::k_document) {
            throw std::runtime_error(std::string{"Expected point document in MongoDB array: "} + fieldName);
        }

        const auto pointDocument = item.get_document().view();
        domain::Point point{};
        point.x = requiredNumber(pointDocument, "x");
        point.y = requiredNumber(pointDocument, "y");
        polygon.push_back(point);
    }

    return polygon;
}

domain::BoundingBox bboxFromDocument(
    bsoncxx::document::view document,
    const char* fieldName
) {
    auto element = requiredElement(document, fieldName);
    if (element.type() != bsoncxx::type::k_document) {
        throw std::runtime_error(std::string{"Expected document MongoDB field: "} + fieldName);
    }

    const auto bboxDocument = element.get_document().view();
    domain::BoundingBox bbox{};
    bbox.x = requiredNumber(bboxDocument, "x");
    bbox.y = requiredNumber(bboxDocument, "y");
    bbox.width = requiredNumber(bboxDocument, "width");
    bbox.height = requiredNumber(bboxDocument, "height");
    return bbox;
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

domain::Zone mapZoneDocument(bsoncxx::document::view document) {
    domain::Zone zone{};
    zone.zoneId = requiredString(document, "zoneId");
    zone.name = requiredString(document, "name");
    zone.cameraId = requiredString(document, "cameraId");
    zone.type = zoneTypeFromString(requiredString(document, "type"));
    zone.polygon = polygonFromDocument(document, "polygon");
    zone.status = zoneStatusFromString(requiredString(document, "status"));
    zone.relatedMachineId = optionalString(document, "relatedMachineId");
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
    alarm.resolvedAt = optionalDate(document, "resolvedAt");
    alarm.message = optionalString(document, "message");
    return alarm;
}

domain::CameraTrack mapCameraTrackDocument(bsoncxx::document::view document) {
    domain::CameraTrack cameraTrack{};
    cameraTrack.trackId = requiredString(document, "trackId");
    cameraTrack.cameraId = requiredString(document, "cameraId");
    cameraTrack.firstSeenAt = requiredDate(document, "firstSeenAt");
    cameraTrack.lastSeenAt = requiredDate(document, "lastSeenAt");
    cameraTrack.currentZoneId = optionalString(document, "currentZoneId");
    cameraTrack.objectClass = requiredString(document, "objectClass");
    cameraTrack.bbox = bboxFromDocument(document, "bbox");
    cameraTrack.status = requiredString(document, "status");
    return cameraTrack;
}

domain::MetadataEvent mapMetadataEventDocument(bsoncxx::document::view document) {
    domain::MetadataEvent metadataEvent{};
    metadataEvent.eventId = requiredString(document, "eventId");
    metadataEvent.cameraId = requiredString(document, "cameraId");
    metadataEvent.trackId = requiredString(document, "trackId");
    metadataEvent.timestamp = requiredDate(document, "timestamp");
    metadataEvent.objectClass = requiredString(document, "objectClass");
    metadataEvent.bbox = bboxFromDocument(document, "bbox");
    metadataEvent.zoneId = optionalString(document, "zoneId");
    metadataEvent.eventType = requiredString(document, "eventType");
    return metadataEvent;
}

domain::TrackIdentityBinding mapTrackIdentityBindingDocument(
    bsoncxx::document::view document
) {
    domain::TrackIdentityBinding binding{};
    binding.bindingId = requiredString(document, "bindingId");
    binding.trackId = requiredString(document, "trackId");
    binding.employeeId = requiredString(document, "employeeId");
    binding.presenceSessionId = requiredString(document, "presenceSessionId");
    binding.confidence = requiredNumber(document, "confidence");
    binding.boundAt = requiredDate(document, "boundAt");
    binding.status = requiredString(document, "status");
    return binding;
}

}
