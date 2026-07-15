#include "securezone/qr/QrCheckInService.h"

#include <chrono>
#include <string>
#include <utility>

namespace securezone::qr {
namespace {

bool hasRequiredFields(const QrCheckInCommand& command) {
    return !command.employeeId.empty()
        && !command.zoneId.empty()
        && !command.scannedByUserId.empty();
}

std::string createCheckInId(
    const QrCheckInCommand& command,
    QrCheckInService::Clock::time_point scannedAt
) {
    const auto unixMillis = std::chrono::duration_cast<std::chrono::milliseconds>(
        scannedAt.time_since_epoch()
    ).count();

    return "CHECKIN-" + command.employeeId + "-" + command.zoneId + "-"
        + std::to_string(unixMillis);
}

QrCheckInResult makeResult(
    const presence::PresenceSessionStartResult& result
) {
    return QrCheckInResult{
        result.accepted(),
        std::string{toQrCheckInStatus(result.status)},
        result.sessionId,
        std::string{toQrCheckInMessage(result.status)},
        {},
        {}
    };
}

}

std::string_view toQrCheckInStatus(
    presence::PresenceSessionStartStatus status
) {
    using Status = presence::PresenceSessionStartStatus;

    switch (status) {
        case Status::Started:
            return "started";
        case Status::Extended:
            return "extended";
        case Status::AlreadyActive:
            return "already_active";
        case Status::InvalidRequest:
            return "invalid_request";
        case Status::EmployeeNotFound:
            return "employee_not_found";
        case Status::EmployeeInactive:
            return "employee_inactive";
        case Status::ZoneNotFound:
            return "zone_not_found";
        case Status::ZoneInactive:
            return "zone_inactive";
        case Status::ScannerNotFound:
            return "scanner_not_found";
        case Status::ScannerNotAllowed:
            return "scanner_not_allowed";
    }

    return "invalid_request";
}

std::string_view toQrCheckInMessage(
    presence::PresenceSessionStartStatus status
) {
    using Status = presence::PresenceSessionStartStatus;

    switch (status) {
        case Status::Started:
        case Status::Extended:
        case Status::AlreadyActive:
            return {};
        case Status::InvalidRequest:
            return "QR check-in request is invalid.";
        case Status::EmployeeNotFound:
            return "Employee was not found.";
        case Status::EmployeeInactive:
            return "Employee is inactive.";
        case Status::ZoneNotFound:
            return "Zone was not found.";
        case Status::ZoneInactive:
            return "Zone is inactive.";
        case Status::ScannerNotFound:
            return "Scanner user was not found.";
        case Status::ScannerNotAllowed:
            return "QR scan must be performed by an active scanner user.";
    }

    return "QR check-in request is invalid.";
}

QrCheckInService::QrCheckInService(
    presence::PresenceSessionService& presenceSessionService,
    NowProvider nowProvider,
    std::chrono::minutes presenceDuration
) : presenceSessionService_{presenceSessionService},
    nowProvider_{std::move(nowProvider)},
    presenceDuration_{presenceDuration} {
}

QrCheckInService::QrCheckInService(
    presence::PresenceSessionService& presenceSessionService,
    identity::CameraIdentityService& cameraIdentityService,
    NowProvider nowProvider,
    std::chrono::minutes presenceDuration
) : presenceSessionService_{presenceSessionService},
    nowProvider_{std::move(nowProvider)},
    presenceDuration_{presenceDuration},
    cameraIdentityService_{&cameraIdentityService} {
}

QrCheckInResult QrCheckInService::checkIn(const QrCheckInCommand& command) {
    if (!hasRequiredFields(command)) {
        return QrCheckInResult{
            false,
            "invalid_request",
            {},
            "employeeId, zoneId and scannedByUserId are required.",
            {},
            {}
        };
    }

    if (cameraIdentityService_ != nullptr && command.cameraId.empty()) {
        return {false, "invalid_request", {}, "cameraId is required for camera identity binding.", {}, {}};
    }

    const auto scannedAt = nowProvider_();

    presence::PresenceSessionStartRequest request{};
    request.checkinId = createCheckInId(command, scannedAt);
    request.employeeId = command.employeeId;
    request.zoneId = command.zoneId;
    request.scannedByUserId = command.scannedByUserId;
    request.scannedAt = scannedAt;
    request.expiresAt = scannedAt + presenceDuration_;

    auto result = makeResult(presenceSessionService_.startFromQrCheckin(request));
    if (!result.accepted || cameraIdentityService_ == nullptr) {
        return result;
    }

    const auto binding = cameraIdentityService_->bindLatestHuman({
        command.cameraId,
        command.employeeId,
        request.checkinId,
        result.sessionId,
        scannedAt,
        request.expiresAt
    });
    if (!binding.bound) {
        if (result.status == "started") {
            presenceSessionService_.endSession(result.sessionId, scannedAt);
        }
        return {false, binding.status, result.sessionId,
            "No recent unbound Human object was found for this camera.", {}, {}};
    }

    result.objectId = binding.objectId;
    result.bindingId = binding.bindingId;
    return result;
}

}
