#include "securezone/presence/PresenceSessionService.h"

#include <chrono>
#include <string>

#include "securezone/domain/Employee.h"
#include "securezone/domain/PresenceSession.h"
#include "securezone/domain/QrCheckin.h"

namespace securezone::presence {

namespace {

using Clock = std::chrono::system_clock;

constexpr const char* ActiveStatus = "active";

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
    qrCheckin.checkinId = request.checkinId;
    qrCheckin.employeeId = request.employeeId;
    qrCheckin.zoneId = request.zoneId;
    qrCheckin.scannedAt = request.scannedAt;
    qrCheckin.expiresAt = request.expiresAt;
    qrCheckin.status = ActiveStatus;
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
    presenceSession.status = ActiveStatus;
    return presenceSession;
}

}

bool PresenceSessionStartResult::accepted() const {
    return status == PresenceSessionStartStatus::Started
        || status == PresenceSessionStartStatus::Extended;
}

PresenceSessionService::PresenceSessionService(
    const repository::IEmployeeRepository& employeeRepository,
    const repository::IZoneRepository& zoneRepository,
    repository::IQrCheckinRepository& qrCheckinRepository,
    repository::IPresenceSessionRepository& presenceSessionRepository
) : employeeRepository_{employeeRepository},
    zoneRepository_{zoneRepository},
    qrCheckinRepository_{qrCheckinRepository},
    presenceSessionRepository_{presenceSessionRepository} {
}

PresenceSessionStartResult PresenceSessionService::startFromQrCheckin(
    const PresenceSessionStartRequest& request
) {
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

    qrCheckinRepository_.create(createQrCheckin(request));

    auto activeSession = presenceSessionRepository_.findActiveByEmployeeAndZone(
        request.employeeId,
        request.zoneId
    );

    if (activeSession.has_value()) {
        presenceSessionRepository_.extend(activeSession->sessionId, request.expiresAt);
        return PresenceSessionStartResult{
            PresenceSessionStartStatus::Extended,
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
