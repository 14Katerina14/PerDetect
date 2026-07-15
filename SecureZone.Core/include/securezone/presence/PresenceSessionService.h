#pragma once

#include <chrono>
#include <string>

#include "securezone/repository/IAppUserRepository.h"
#include "securezone/repository/IEmployeeRepository.h"
#include "securezone/repository/IPresenceSessionRepository.h"
#include "securezone/repository/IQrCheckinRepository.h"
#include "securezone/repository/IZoneRepository.h"

namespace securezone::presence {

enum class PresenceSessionStartStatus {
    Started,
    Extended,
    AlreadyActive,
    InvalidRequest,
    EmployeeNotFound,
    EmployeeInactive,
    ZoneNotFound,
    ZoneInactive,
    ScannerNotFound,
    ScannerNotAllowed
};

struct PresenceSessionStartRequest {
    std::string checkinId;
    std::string employeeId;
    std::string zoneId;
    std::string scannedByUserId;
    std::chrono::system_clock::time_point scannedAt;
    std::chrono::system_clock::time_point expiresAt;
};

struct PresenceSessionStartResult {
    PresenceSessionStartStatus status{};
    std::string sessionId;

    bool accepted() const;
};

class PresenceSessionService {
public:
    PresenceSessionService(
        const repository::IEmployeeRepository& employeeRepository,
        const repository::IAppUserRepository& appUserRepository,
        const repository::IZoneRepository& zoneRepository,
        repository::IQrCheckinRepository& qrCheckinRepository,
        repository::IPresenceSessionRepository& presenceSessionRepository
    );

    PresenceSessionStartResult startFromQrCheckin(
        const PresenceSessionStartRequest& request
    );

    void endSession(
        const std::string& sessionId,
        std::chrono::system_clock::time_point endedAt
    );

private:
    const repository::IEmployeeRepository& employeeRepository_;
    const repository::IAppUserRepository& appUserRepository_;
    const repository::IZoneRepository& zoneRepository_;
    repository::IQrCheckinRepository& qrCheckinRepository_;
    repository::IPresenceSessionRepository& presenceSessionRepository_;
};

}
