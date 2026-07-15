#include "securezone/qr/QrCheckInService.h"

#include "securezone/domain/AppUser.h"
#include "securezone/domain/Employee.h"
#include "securezone/domain/PresenceSession.h"
#include "securezone/domain/QrCheckIn.h"
#include "securezone/domain/Zone.h"
#include "securezone/repository/IAppUserRepository.h"
#include "securezone/repository/IEmployeeRepository.h"
#include "securezone/repository/IPresenceSessionRepository.h"
#include "securezone/repository/IQrCheckinRepository.h"
#include "securezone/repository/IZoneRepository.h"

#include <cassert>
#include <chrono>
#include <optional>
#include <string>
#include <vector>

namespace {

using namespace securezone;

using Clock = std::chrono::system_clock;
const Clock::time_point FixedNow{std::chrono::milliseconds{1784123456789}};

domain::Employee activeEmployee() {
    domain::Employee employee{};
    employee.employeeId = "EMP-001";
    employee.fullName = "Ivan Petrov";
    employee.roles = {"maintenance"};
    employee.status = domain::EmployeeStatus::Active;
    return employee;
}

domain::AppUser activeScanner() {
    domain::AppUser user{};
    user.userId = "APP-SCANNER-001";
    user.username = "scanner";
    user.role = domain::AppUserRole::Scanner;
    user.status = domain::AppUserStatus::Active;
    return user;
}

domain::Zone activeZone() {
    domain::Zone zone{};
    zone.zoneId = "ZONE-001";
    zone.name = "Machine A Dangerous Zone";
    zone.cameraId = "CAM-001";
    zone.type = domain::ZoneType::Dangerous;
    zone.status = domain::ZoneStatus::Active;
    return zone;
}

qr::QrCheckInCommand validCommand() {
    return {"EMP-001", "ZONE-001", "APP-SCANNER-001"};
}

class FakeEmployeeRepository final : public repository::IEmployeeRepository {
public:
    std::optional<domain::Employee> employee;

    std::optional<domain::Employee> findByEmployeeId(
        const std::string& employeeId
    ) const override {
        if (employee && employee->employeeId == employeeId) {
            return employee;
        }

        return std::nullopt;
    }
};

class FakeAppUserRepository final : public repository::IAppUserRepository {
public:
    std::optional<domain::AppUser> user;

    std::optional<domain::AppUser> findByUserId(
        const std::string& userId
    ) const override {
        if (user && user->userId == userId) {
            return user;
        }

        return std::nullopt;
    }

    std::optional<domain::AppUser> findByUsername(
        const std::string& username
    ) const override {
        if (user && user->username == username) {
            return user;
        }

        return std::nullopt;
    }
};

class FakeZoneRepository final : public repository::IZoneRepository {
public:
    std::optional<domain::Zone> zone;

    std::optional<domain::Zone> findByZoneId(
        const std::string& zoneId
    ) const override {
        if (zone && zone->zoneId == zoneId) {
            return zone;
        }

        return std::nullopt;
    }

    std::optional<domain::Zone> findActiveByZoneId(
        const std::string& zoneId
    ) const override {
        auto result = findByZoneId(zoneId);
        if (result && result->status == domain::ZoneStatus::Active) {
            return result;
        }

        return std::nullopt;
    }

    bool save(const domain::Zone& value) override {
        zone = value;
        return true;
    }
};

class FakeQrCheckinRepository final : public repository::IQrCheckinRepository {
public:
    std::vector<domain::QrCheckin> created;

    void create(const domain::QrCheckin& checkin) override {
        created.push_back(checkin);
    }

    std::optional<domain::QrCheckin> findByCheckinId(
        const std::string& checkinId
    ) const override {
        for (const auto& checkin : created) {
            if (checkin.checkInId == checkinId) {
                return checkin;
            }
        }

        return std::nullopt;
    }

    std::optional<domain::QrCheckin> findLatestActiveByEmployeeAndZone(
        const std::string& employeeId,
        const std::string& zoneId
    ) const override {
        for (auto iterator = created.rbegin(); iterator != created.rend(); ++iterator) {
            if (iterator->employeeId == employeeId
                && iterator->zoneId == zoneId
                && iterator->status == domain::QrCheckInStatus::Active) {
                return *iterator;
            }
        }

        return std::nullopt;
    }
};

class FakePresenceSessionRepository final : public repository::IPresenceSessionRepository {
public:
    std::optional<domain::PresenceSession> activeSession;
    std::vector<domain::PresenceSession> created;
    std::string extendedSessionId;
    Clock::time_point extendedUntil{};
    std::string endedSessionId;
    Clock::time_point endedAt{};

    std::optional<domain::PresenceSession> findActiveByEmployeeAndZone(
        const std::string& employeeId,
        const std::string& zoneId
    ) const override {
        if (activeSession
            && activeSession->employeeId == employeeId
            && activeSession->zoneId == zoneId) {
            return activeSession;
        }

        return std::nullopt;
    }

    void create(const domain::PresenceSession& session) override {
        created.push_back(session);
    }

    void extend(
        const std::string& sessionId,
        Clock::time_point expiresAt
    ) override {
        extendedSessionId = sessionId;
        extendedUntil = expiresAt;
    }

    void end(const std::string& sessionId, Clock::time_point value) override {
        endedSessionId = sessionId;
        endedAt = value;
    }
};

struct TestContext {
    FakeEmployeeRepository employees;
    FakeAppUserRepository appUsers;
    FakeZoneRepository zones;
    FakeQrCheckinRepository checkins;
    FakePresenceSessionRepository sessions;
    presence::PresenceSessionService presenceService;
    qr::QrCheckInService qrService;

    TestContext()
        : presenceService{employees, appUsers, zones, checkins, sessions},
          qrService{presenceService, [] { return FixedNow; }} {
        employees.employee = activeEmployee();
        appUsers.user = activeScanner();
        zones.zone = activeZone();
    }
};

void validCommandStartsPresenceSession() {
    TestContext context;

    const auto result = context.qrService.checkIn(validCommand());

    assert(result.accepted);
    assert(result.status == "started");
    assert(result.message.empty());
    assert(!result.sessionId.empty());
    assert(context.checkins.created.size() == 1);
    assert(context.checkins.created.front().checkInId == "CHECKIN-EMP-001-ZONE-001-1784123456789");
    assert(context.checkins.created.front().validUntil == FixedNow + std::chrono::minutes{2});
    assert(context.checkins.created.front().scannedByUserId == "APP-SCANNER-001");
    assert(context.sessions.created.size() == 1);
}

void missingRequiredFieldsAreRejectedBeforeCallingPresenceService() {
    const std::vector<qr::QrCheckInCommand> invalidCommands{
        {"", "ZONE-001", "APP-SCANNER-001"},
        {"EMP-001", "", "APP-SCANNER-001"},
        {"EMP-001", "ZONE-001", ""}
    };

    for (const auto& command : invalidCommands) {
        TestContext context;

        const auto result = context.qrService.checkIn(command);

        assert(!result.accepted);
        assert(result.status == "invalid_request");
        assert(result.message == "employeeId, zoneId and scannedByUserId are required.");
        assert(context.checkins.created.empty());
        assert(context.sessions.created.empty());
    }
}

void scannerMustExistAndHaveScannerRole() {
    TestContext missingScanner;
    missingScanner.appUsers.user.reset();

    const auto missingResult = missingScanner.qrService.checkIn(validCommand());

    assert(!missingResult.accepted);
    assert(missingResult.status == "scanner_not_found");
    assert(missingResult.message == "Scanner user was not found.");

    TestContext wrongRole;
    wrongRole.appUsers.user->role = domain::AppUserRole::Manager;

    const auto wrongRoleResult = wrongRole.qrService.checkIn(validCommand());

    assert(!wrongRoleResult.accepted);
    assert(wrongRoleResult.status == "scanner_not_allowed");
    assert(wrongRoleResult.message == "QR scan must be performed by an active scanner user.");
}

void inactiveEmployeeAndZoneAreMappedToMessages() {
    TestContext inactiveEmployee;
    inactiveEmployee.employees.employee->status = domain::EmployeeStatus::Inactive;

    const auto employeeResult = inactiveEmployee.qrService.checkIn(validCommand());

    assert(!employeeResult.accepted);
    assert(employeeResult.status == "employee_inactive");
    assert(employeeResult.message == "Employee is inactive.");

    TestContext inactiveZone;
    inactiveZone.zones.zone->status = domain::ZoneStatus::Inactive;

    const auto zoneResult = inactiveZone.qrService.checkIn(validCommand());

    assert(!zoneResult.accepted);
    assert(zoneResult.status == "zone_inactive");
    assert(zoneResult.message == "Zone is inactive.");
}

void activeSessionIsExtended() {
    TestContext context;
    domain::PresenceSession session{};
    session.sessionId = "SESSION-001";
    session.employeeId = "EMP-001";
    session.zoneId = "ZONE-001";
    session.sourceCheckinId = "CHECKIN-OLD";
    session.startedAt = FixedNow - std::chrono::minutes{1};
    session.expiresAt = FixedNow + std::chrono::minutes{1};
    session.status = domain::PresenceSessionStatus::Active;
    context.sessions.activeSession = session;

    const auto result = context.qrService.checkIn(validCommand());

    assert(result.accepted);
    assert(result.status == "extended");
    assert(result.sessionId == "SESSION-001");
    assert(context.sessions.extendedSessionId == "SESSION-001");
    assert(context.sessions.extendedUntil == FixedNow + std::chrono::minutes{2});
}

void statusMappingCoversPresenceSessionResults() {
    using Status = presence::PresenceSessionStartStatus;
    assert(qr::toQrCheckInStatus(Status::Started) == "started");
    assert(qr::toQrCheckInStatus(Status::Extended) == "extended");
    assert(qr::toQrCheckInStatus(Status::AlreadyActive) == "already_active");
    assert(qr::toQrCheckInStatus(Status::InvalidRequest) == "invalid_request");
    assert(qr::toQrCheckInStatus(Status::EmployeeNotFound) == "employee_not_found");
    assert(qr::toQrCheckInStatus(Status::EmployeeInactive) == "employee_inactive");
    assert(qr::toQrCheckInStatus(Status::ZoneNotFound) == "zone_not_found");
    assert(qr::toQrCheckInStatus(Status::ZoneInactive) == "zone_inactive");
    assert(qr::toQrCheckInStatus(Status::ScannerNotFound) == "scanner_not_found");
    assert(qr::toQrCheckInStatus(Status::ScannerNotAllowed) == "scanner_not_allowed");
    assert(qr::toQrCheckInMessage(Status::Started).empty());
    assert(qr::toQrCheckInMessage(Status::Extended).empty());
    assert(qr::toQrCheckInMessage(Status::AlreadyActive).empty());
}

}

int main() {
    validCommandStartsPresenceSession();
    missingRequiredFieldsAreRejectedBeforeCallingPresenceService();
    scannerMustExistAndHaveScannerRole();
    inactiveEmployeeAndZoneAreMappedToMessages();
    activeSessionIsExtended();
    statusMappingCoversPresenceSessionResults();
}
