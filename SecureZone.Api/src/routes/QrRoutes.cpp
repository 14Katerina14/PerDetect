#include "securezone/api/routes/QrRoutes.h"

#include <chrono>
#include <string>
#include <utility>

namespace securezone::api::routes {

namespace {

constexpr int HttpOk = 200;
constexpr int HttpCreated = 201;
constexpr int HttpBadRequest = 400;
constexpr int HttpForbidden = 403;
constexpr int HttpNotFound = 404;
constexpr auto PresenceDuration = std::chrono::minutes{2};

bool hasRequiredFields(const QrCheckInRequest& request) {
    return !request.employeeId.empty()
        && !request.zoneId.empty()
        && !request.scannedByUserId.empty();
}

std::string createCheckinId(
    const QrCheckInRequest& request,
    QrRoutes::Clock::time_point scannedAt
) {
    const auto unixMillis = std::chrono::duration_cast<std::chrono::milliseconds>(
        scannedAt.time_since_epoch()
    ).count();

    return "CHECKIN-" + request.employeeId + "-" + request.zoneId + "-"
        + std::to_string(unixMillis);
}

int toHttpStatus(presence::PresenceSessionStartStatus status) {
    using Status = presence::PresenceSessionStartStatus;

    switch (status) {
        case Status::Started:
            return HttpCreated;
        case Status::Extended:
        case Status::AlreadyActive:
            return HttpOk;
        case Status::InvalidRequest:
            return HttpBadRequest;
        case Status::EmployeeNotFound:
        case Status::ZoneNotFound:
        case Status::ScannerNotFound:
            return HttpNotFound;
        case Status::EmployeeInactive:
        case Status::ZoneInactive:
        case Status::ScannerNotAllowed:
            return HttpForbidden;
    }

    return HttpBadRequest;
}

QrCheckInResponse makeResponse(
    const presence::PresenceSessionStartResult& result
) {
    return QrCheckInResponse{
        toHttpStatus(result.status),
        result.accepted(),
        std::string{toQrApiStatus(result.status)},
        result.sessionId,
        std::string{toQrApiMessage(result.status)}
    };
}

}

std::string_view toQrApiStatus(
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

std::string_view toQrApiMessage(
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

QrRoutes::QrRoutes(
    presence::PresenceSessionService& presenceSessionService,
    NowProvider nowProvider
) : presenceSessionService_{presenceSessionService},
    nowProvider_{std::move(nowProvider)} {
}

QrCheckInResponse QrRoutes::handleCheckIn(const QrCheckInRequest& request) {
    if (!hasRequiredFields(request)) {
        return QrCheckInResponse{
            HttpBadRequest,
            false,
            "invalid_request",
            {},
            "employeeId, zoneId and scannedByUserId are required."
        };
    }

    const auto scannedAt = nowProvider_();
    presence::PresenceSessionStartRequest serviceRequest{};
    serviceRequest.checkinId = createCheckinId(request, scannedAt);
    serviceRequest.employeeId = request.employeeId;
    serviceRequest.zoneId = request.zoneId;
    serviceRequest.scannedByUserId = request.scannedByUserId;
    serviceRequest.scannedAt = scannedAt;
    serviceRequest.expiresAt = scannedAt + PresenceDuration;

    return makeResponse(
        presenceSessionService_.startFromQrCheckin(serviceRequest)
    );
}

}
