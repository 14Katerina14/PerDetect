#include "securezone/presence/PresenceSessionService.h"

#include <chrono>
#include <string>

#include "securezone/domain/AppUser.h"
#include "securezone/domain/Employee.h"
#include "securezone/domain/PresenceSession.h"
#include "securezone/domain/QrCheckIn.h"

namespace securezone::presence {

namespace {

using Clock = std::chrono::system_clock;

std::string timestampSuffix(Clock::time_point timePoint) {
    const auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(
        timePoint.time_since_epoch()
    ).count();

    return std::to_string(milliseconds);
}

std::string createSessionId(
    const std::string& employeeId,
    const std::string& zoneId,
    Clock::time_point startedAt
) {
    return "SESSION-" + employeeId + "-" + zoneId + "-" + timestampSuffix(startedAt);
}

domain::QrCheckin createQrCheckin(
    const PresenceSessionStartRequest& request
) {
    domain::QrCheckin qrCheckin{};
    qrCheckin.checkInId = request.checkinId;
    qrCheckin.employeeId = request.employeeId;
    qrCheckin.zoneId = request.zoneId;
    qrCheckin.scannedByUserId = request.scannedByUserId;
    qrCheckin.scannedAt = request.scannedAt;
    qrCheckin.validUntil = request.expiresAt;
    qrCheckin.status = domain::QrCheckInStatus::Active;
    return qrCheckin;
}

domain::PresenceSession createPresenceSession(
    const PresenceSessionStartRequest& request
) {
    domain::PresenceSession presenceSession{};
    presenceSession.sessionId = createSessionId(
        request.employeeId,
        request.zoneId,
        request.scannedAt
    );
    presenceSession.employeeId = request.employeeId;
    presenceSession.zoneId = request.zoneId;
    presenceSession.sourceCheckinId = request.checkinId;
    presenceSession.startedAt = request.scannedAt;
    presenceSession.expiresAt = request.expiresAt;
    presenceSession.status = domain::PresenceSessionStatus::Active;
    return presenceSession;
}

}

bool PresenceSessionStartResult::accepted() const {
    return status == PresenceSessionStartStatus::Started
        || status == PresenceSessionStartStatus::Extended
        || status == PresenceSessionStartStatus::AlreadyActive;
}

PresenceSessionService::PresenceSessionService(
    const repository::IEmployeeRepository& employeeRepository,
    const repository::IAppUserRepository& appUserRepository,
    const repository::IZoneRepository& zoneRepository,
    repository::IQrCheckinRepository& qrCheckinRepository,
    repository::IPresenceSessionRepository& presenceSessionRepository
) : employeeRepository_{employeeRepository},
    appUserRepository_{appUserRepository},
    zoneRepository_{zoneRepository},
    qrCheckinRepository_{qrCheckinRepository},
    presenceSessionRepository_{presenceSessionRepository} {
}

PresenceSessionStartResult PresenceSessionService::startFromQrCheckin(
    const PresenceSessionStartRequest& request
) {
    if (request.checkinId.empty()
        || request.employeeId.empty()
        || request.zoneId.empty()
        || request.scannedByUserId.empty()
        || request.expiresAt <= request.scannedAt) {
        return PresenceSessionStartResult{
            PresenceSessionStartStatus::InvalidRequest,
            {}
        };
    }

    auto employee = employeeRepository_.findByEmployeeId(request.employeeId);
    if (!employee.has_value()) {
        return PresenceSessionStartResult{
            PresenceSessionStartStatus::EmployeeNotFound,
            {}
        };
    }

    if (employee->status != domain::EmployeeStatus::Active) {
        return PresenceSessionStartResult{
            PresenceSessionStartStatus::EmployeeInactive,
            {}
        };
    }

    auto zone = zoneRepository_.findByZoneId(request.zoneId);
    if (!zone.has_value()) {
        return PresenceSessionStartResult{
            PresenceSessionStartStatus::ZoneNotFound,
            {}
        };
    }

    if (zone->status != domain::ZoneStatus::Active) {
        return PresenceSessionStartResult{
            PresenceSessionStartStatus::ZoneInactive,
            {}
        };
    }

    auto scanner = appUserRepository_.findByUserId(request.scannedByUserId);
    if (!scanner.has_value()) {
        return PresenceSessionStartResult{
            PresenceSessionStartStatus::ScannerNotFound,
            {}
        };
    }

    if (!scanner->canScanQr()) {
        return PresenceSessionStartResult{
            PresenceSessionStartStatus::ScannerNotAllowed,
            {}
        };
    }

    qrCheckinRepository_.create(createQrCheckin(request));

    auto activeSession = presenceSessionRepository_.findActiveByEmployeeAndZone(
        request.employeeId,
        request.zoneId
    );

    if (activeSession.has_value()) {
        if (activeSession->canExtendTo(request.expiresAt)) {
            presenceSessionRepository_.extend(activeSession->sessionId, request.expiresAt);
            return PresenceSessionStartResult{
                PresenceSessionStartStatus::Extended,
                activeSession->sessionId
            };
        }

        return PresenceSessionStartResult{
            PresenceSessionStartStatus::AlreadyActive,
            activeSession->sessionId
        };
    }

    auto presenceSession = createPresenceSession(request);
    const auto sessionId = presenceSession.sessionId;
    presenceSessionRepository_.create(presenceSession);

    return PresenceSessionStartResult{
        PresenceSessionStartStatus::Started,
        sessionId
    };
}

void PresenceSessionService::endSession(
    const std::string& sessionId,
    Clock::time_point endedAt
) {
    presenceSessionRepository_.end(sessionId, endedAt);
}

}
