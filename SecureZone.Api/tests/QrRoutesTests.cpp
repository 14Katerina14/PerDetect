#include <cassert>
#include <chrono>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "securezone/api/routes/QrRoutes.h"
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

namespace {

using Clock = std::chrono::system_clock;
using TimePoint = Clock::time_point;
namespace api = securezone::api::routes;
namespace domain = securezone::domain;
namespace presence = securezone::presence;
namespace repository = securezone::repository;

const TimePoint FixedNow{std::chrono::milliseconds{1784123456789}};

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

class FakePresenceSessionRepository final
    : public repository::IPresenceSessionRepository {
public:
    std::optional<domain::PresenceSession> activeSession;
    std::vector<domain::PresenceSession> created;
    std::string extendedSessionId;
    TimePoint extendedUntil{};
    std::string endedSessionId;
    TimePoint endedAt{};

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
        TimePoint expiresAt
    ) override {
        extendedSessionId = sessionId;
        extendedUntil = expiresAt;
    }

    void end(const std::string& sessionId, TimePoint value) override {
        endedSessionId = sessionId;
        endedAt = value;
    }
};

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
    zone.xprotectEventName = "SecureZone.LineCrossing";
    return zone;
}

api::QrCheckInRequest validRequest() {
    return {"EMP-001", "ZONE-001", "APP-SCANNER-001"};
}

struct TestContext {
    FakeEmployeeRepository employees;
    FakeAppUserRepository appUsers;
    FakeZoneRepository zones;
    FakeQrCheckinRepository checkins;
    FakePresenceSessionRepository sessions;
    presence::PresenceSessionService service;
    api::QrRoutes routes;

    TestContext()
        : service{employees, appUsers, zones, checkins, sessions},
          routes{service, [] { return FixedNow; }} {
        employees.employee = activeEmployee();
        appUsers.user = activeScanner();
        zones.zone = activeZone();
    }
};

void validRequestStartsPresenceSession() {
    TestContext context;

    const auto response = context.routes.handleCheckIn(validRequest());

    assert(response.statusCode == 201);
    assert(response.accepted);
    assert(response.status == "started");
    assert(response.message.empty());
    assert(!response.sessionId.empty());
    assert(context.checkins.created.size() == 1);
    const auto& checkin = context.checkins.created.front();
    assert(checkin.checkInId == "CHECKIN-EMP-001-ZONE-001-1784123456789");
    assert(checkin.scannedAt == FixedNow);
    assert(checkin.validUntil == FixedNow + std::chrono::minutes{2});
    assert(checkin.scannedByUserId == "APP-SCANNER-001");
    assert(context.sessions.created.size() == 1);
}

void missingRequiredFieldsReturnBadRequest() {
    const std::vector<api::QrCheckInRequest> invalidRequests{
        {"", "ZONE-001", "APP-SCANNER-001"},
        {"EMP-001", "", "APP-SCANNER-001"},
        {"EMP-001", "ZONE-001", ""}
    };

    for (const auto& request : invalidRequests) {
        TestContext context;
        const auto response = context.routes.handleCheckIn(request);
        assert(response.statusCode == 400);
        assert(!response.accepted);
        assert(response.status == "invalid_request");
        assert(response.message == "employeeId, zoneId and scannedByUserId are required.");
        assert(context.checkins.created.empty());
        assert(context.sessions.created.empty());
    }
}

void scannerNotFoundIsMapped() {
    TestContext context;
    context.appUsers.user.reset();

    const auto response = context.routes.handleCheckIn(validRequest());

    assert(response.statusCode == 404);
    assert(!response.accepted);
    assert(response.status == "scanner_not_found");
    assert(response.message == "Scanner user was not found.");
}

void scannerNotAllowedIsMapped() {
    TestContext context;
    context.appUsers.user->role = domain::AppUserRole::Manager;

    const auto response = context.routes.handleCheckIn(validRequest());

    assert(response.statusCode == 403);
    assert(!response.accepted);
    assert(response.status == "scanner_not_allowed");
    assert(response.message == "QR scan must be performed by an active scanner user.");
}

void employeeNotFoundIsMapped() {
    TestContext context;
    context.employees.employee.reset();

    const auto response = context.routes.handleCheckIn(validRequest());

    assert(response.statusCode == 404);
    assert(!response.accepted);
    assert(response.status == "employee_not_found");
    assert(response.message == "Employee was not found.");
}

void inactiveEmployeeIsMapped() {
    TestContext context;
    context.employees.employee->status = domain::EmployeeStatus::Inactive;

    const auto response = context.routes.handleCheckIn(validRequest());

    assert(response.statusCode == 403);
    assert(!response.accepted);
    assert(response.status == "employee_inactive");
    assert(response.message == "Employee is inactive.");
}

void zoneNotFoundIsMapped() {
    TestContext context;
    context.zones.zone.reset();

    const auto response = context.routes.handleCheckIn(validRequest());

    assert(response.statusCode == 404);
    assert(!response.accepted);
    assert(response.status == "zone_not_found");
    assert(response.message == "Zone was not found.");
}

void inactiveZoneIsMapped() {
    TestContext context;
    context.zones.zone->status = domain::ZoneStatus::Inactive;

    const auto response = context.routes.handleCheckIn(validRequest());

    assert(response.statusCode == 403);
    assert(!response.accepted);
    assert(response.status == "zone_inactive");
    assert(response.message == "Zone is inactive.");
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

    const auto response = context.routes.handleCheckIn(validRequest());

    assert(response.statusCode == 200);
    assert(response.accepted);
    assert(response.status == "extended");
    assert(response.sessionId == "SESSION-001");
    assert(context.sessions.extendedSessionId == "SESSION-001");
    assert(context.sessions.extendedUntil == FixedNow + std::chrono::minutes{2});
}

void longerActiveSessionStaysActive() {
    TestContext context;
    domain::PresenceSession session{};
    session.sessionId = "SESSION-001";
    session.employeeId = "EMP-001";
    session.zoneId = "ZONE-001";
    session.sourceCheckinId = "CHECKIN-OLD";
    session.startedAt = FixedNow - std::chrono::minutes{1};
    session.expiresAt = FixedNow + std::chrono::minutes{5};
    session.status = domain::PresenceSessionStatus::Active;
    context.sessions.activeSession = session;

    const auto response = context.routes.handleCheckIn(validRequest());

    assert(response.statusCode == 200);
    assert(response.accepted);
    assert(response.status == "already_active");
    assert(response.sessionId == "SESSION-001");
    assert(context.sessions.extendedSessionId.empty());
}

void everyDomainStatusHasExpectedApiMapping() {
    using Status = presence::PresenceSessionStartStatus;
    assert(api::toQrApiStatus(Status::Started) == "started");
    assert(api::toQrApiStatus(Status::Extended) == "extended");
    assert(api::toQrApiStatus(Status::AlreadyActive) == "already_active");
    assert(api::toQrApiStatus(Status::InvalidRequest) == "invalid_request");
    assert(api::toQrApiStatus(Status::EmployeeNotFound) == "employee_not_found");
    assert(api::toQrApiStatus(Status::EmployeeInactive) == "employee_inactive");
    assert(api::toQrApiStatus(Status::ZoneNotFound) == "zone_not_found");
    assert(api::toQrApiStatus(Status::ZoneInactive) == "zone_inactive");
    assert(api::toQrApiStatus(Status::ScannerNotFound) == "scanner_not_found");
    assert(api::toQrApiStatus(Status::ScannerNotAllowed) == "scanner_not_allowed");
    assert(api::toQrApiMessage(Status::Started).empty());
    assert(api::toQrApiMessage(Status::Extended).empty());
    assert(api::toQrApiMessage(Status::AlreadyActive).empty());
    assert(api::toQrApiMessage(Status::InvalidRequest) == "QR check-in request is invalid.");
    assert(api::toQrApiMessage(Status::EmployeeNotFound) == "Employee was not found.");
    assert(api::toQrApiMessage(Status::EmployeeInactive) == "Employee is inactive.");
    assert(api::toQrApiMessage(Status::ZoneNotFound) == "Zone was not found.");
    assert(api::toQrApiMessage(Status::ZoneInactive) == "Zone is inactive.");
    assert(api::toQrApiMessage(Status::ScannerNotFound) == "Scanner user was not found.");
    assert(api::toQrApiMessage(Status::ScannerNotAllowed)
        == "QR scan must be performed by an active scanner user.");
}

void exposesExpectedEndpointContract() {
    assert(api::QrRoutes::CheckInMethod == "POST");
    assert(api::QrRoutes::CheckInPath == "/api/qr/check-in");
}

}

int main() {
    validRequestStartsPresenceSession();
    missingRequiredFieldsReturnBadRequest();
    scannerNotFoundIsMapped();
    scannerNotAllowedIsMapped();
    employeeNotFoundIsMapped();
    inactiveEmployeeIsMapped();
    zoneNotFoundIsMapped();
    inactiveZoneIsMapped();
    activeSessionIsExtended();
    longerActiveSessionStaysActive();
    everyDomainStatusHasExpectedApiMapping();
    exposesExpectedEndpointContract();
}
