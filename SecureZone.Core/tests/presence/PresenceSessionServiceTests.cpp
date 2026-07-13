#include "securezone/presence/PresenceSessionService.h"

#include <cassert>
#include <chrono>
#include <optional>
#include <string>
#include <vector>

namespace {

using namespace securezone;

using Clock = std::chrono::system_clock;

Clock::time_point testTime(int offsetSeconds = 0) {
    return Clock::time_point{} + std::chrono::seconds{offsetSeconds};
}

domain::Employee makeEmployee(
    domain::EmployeeStatus status = domain::EmployeeStatus::Active
) {
    domain::Employee employee{};
    employee.employeeId = "EMP-001";
    employee.fullName = "Ivan Petrov";
    employee.department = "Maintenance";
    employee.roles = {"maintenance"};
    employee.status = status;
    return employee;
}

domain::Zone makeZone() {
    domain::Zone zone{};
    zone.zoneId = "ZONE-001";
    zone.name = "Machine A Dangerous Zone";
    zone.cameraId = "CAM-001";
    zone.type = domain::ZoneType::Dangerous;
    zone.status = domain::ZoneStatus::Active;
    zone.relatedMachineId = "MACHINE-001";
    return zone;
}

domain::PresenceSession makePresenceSession() {
    domain::PresenceSession session{};
    session.sessionId = "SESSION-001";
    session.employeeId = "EMP-001";
    session.zoneId = "ZONE-001";
    session.sourceCheckinId = "CHECKIN-001";
    session.startedAt = testTime(10);
    session.expiresAt = testTime(70);
    session.status = domain::PresenceSessionStatus::Active;
    return session;
}

presence::PresenceSessionStartRequest makeRequest() {
    presence::PresenceSessionStartRequest request{};
    request.checkinId = "CHECKIN-001";
    request.employeeId = "EMP-001";
    request.zoneId = "ZONE-001";
    request.scannedAt = testTime(10);
    request.expiresAt = testTime(70);
    return request;
}

class FakeEmployeeRepository final : public repository::IEmployeeRepository {
public:
    std::optional<domain::Employee> employee;

    std::optional<domain::Employee> findByEmployeeId(
        const std::string& employeeId
    ) const override {
        if (!employee.has_value() || employee->employeeId != employeeId) {
            return std::nullopt;
        }

        return employee;
    }
};

class FakeZoneRepository final : public repository::IZoneRepository {
public:
    std::optional<domain::Zone> zone;

    std::optional<domain::Zone> findByZoneId(
        const std::string& zoneId
    ) const override {
        if (!zone.has_value() || zone->zoneId != zoneId) {
            return std::nullopt;
        }

        return zone;
    }

    std::optional<domain::Zone> findActiveByZoneId(
        const std::string& zoneId
    ) const override {
        if (!zone.has_value() || zone->zoneId != zoneId) {
            return std::nullopt;
        }

        if (zone->status != domain::ZoneStatus::Active) {
            return std::nullopt;
        }

        return zone;
    }
};

class FakeQrCheckinRepository final : public repository::IQrCheckinRepository {
public:
    std::vector<domain::QrCheckin> createdCheckins;

    void create(const domain::QrCheckin& qrCheckin) override {
        createdCheckins.push_back(qrCheckin);
    }

    std::optional<domain::QrCheckin> findByCheckinId(
        const std::string& checkinId
    ) const override {
        for (const auto& checkin : createdCheckins) {
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
        for (auto iterator = createdCheckins.rbegin(); iterator != createdCheckins.rend(); ++iterator) {
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
    std::vector<domain::PresenceSession> createdSessions;
    std::string extendedSessionId;
    Clock::time_point extendedExpiresAt{};
    std::string endedSessionId;
    Clock::time_point endedAt{};

    std::optional<domain::PresenceSession> findActiveByEmployeeAndZone(
        const std::string& employeeId,
        const std::string& zoneId
    ) const override {
        if (!activeSession.has_value()) {
            return std::nullopt;
        }

        if (activeSession->employeeId != employeeId || activeSession->zoneId != zoneId) {
            return std::nullopt;
        }

        return activeSession;
    }

    void create(const domain::PresenceSession& presenceSession) override {
        createdSessions.push_back(presenceSession);
    }

    void extend(
        const std::string& sessionId,
        Clock::time_point expiresAt
    ) override {
        extendedSessionId = sessionId;
        extendedExpiresAt = expiresAt;
    }

    void end(
        const std::string& sessionId,
        Clock::time_point value
    ) override {
        endedSessionId = sessionId;
        endedAt = value;
    }
};

struct TestContext {
    FakeEmployeeRepository employeeRepository;
    FakeZoneRepository zoneRepository;
    FakeQrCheckinRepository qrCheckinRepository;
    FakePresenceSessionRepository presenceSessionRepository;
};

presence::PresenceSessionService makeService(TestContext& context) {
    return presence::PresenceSessionService{
        context.employeeRepository,
        context.zoneRepository,
        context.qrCheckinRepository,
        context.presenceSessionRepository
    };
}

void rejectsMissingEmployeeWithoutCreatingCheckin() {
    TestContext context{};
    context.zoneRepository.zone = makeZone();
    auto service = makeService(context);

    const auto result = service.startFromQrCheckin(makeRequest());

    assert(result.status == presence::PresenceSessionStartStatus::EmployeeNotFound);
    assert(!result.accepted());
    assert(context.qrCheckinRepository.createdCheckins.empty());
    assert(context.presenceSessionRepository.createdSessions.empty());
}

void rejectsInactiveEmployeeWithoutCreatingCheckin() {
    TestContext context{};
    context.employeeRepository.employee = makeEmployee(domain::EmployeeStatus::Inactive);
    context.zoneRepository.zone = makeZone();
    auto service = makeService(context);

    const auto result = service.startFromQrCheckin(makeRequest());

    assert(result.status == presence::PresenceSessionStartStatus::EmployeeInactive);
    assert(!result.accepted());
    assert(context.qrCheckinRepository.createdCheckins.empty());
    assert(context.presenceSessionRepository.createdSessions.empty());
}

void rejectsMissingZoneWithoutCreatingCheckin() {
    TestContext context{};
    context.employeeRepository.employee = makeEmployee();
    auto service = makeService(context);

    const auto result = service.startFromQrCheckin(makeRequest());

    assert(result.status == presence::PresenceSessionStartStatus::ZoneNotFound);
    assert(!result.accepted());
    assert(context.qrCheckinRepository.createdCheckins.empty());
    assert(context.presenceSessionRepository.createdSessions.empty());
}

void createsNewPresenceSessionWhenNoActiveSessionExists() {
    TestContext context{};
    context.employeeRepository.employee = makeEmployee();
    context.zoneRepository.zone = makeZone();
    auto service = makeService(context);

    const auto result = service.startFromQrCheckin(makeRequest());

    assert(result.status == presence::PresenceSessionStartStatus::Started);
    assert(result.accepted());
    assert(!result.sessionId.empty());
    assert(context.qrCheckinRepository.createdCheckins.size() == 1);
    assert(context.presenceSessionRepository.createdSessions.size() == 1);
    assert(context.presenceSessionRepository.createdSessions.front().sessionId == result.sessionId);
    assert(context.presenceSessionRepository.createdSessions.front().sourceCheckinId == "CHECKIN-001");
}

void extendsExistingPresenceSessionWhenActiveSessionExists() {
    TestContext context{};
    context.employeeRepository.employee = makeEmployee();
    context.zoneRepository.zone = makeZone();
    context.presenceSessionRepository.activeSession = makePresenceSession();
    auto service = makeService(context);

    const auto request = makeRequest();
    const auto result = service.startFromQrCheckin(request);

    assert(result.status == presence::PresenceSessionStartStatus::Extended);
    assert(result.accepted());
    assert(result.sessionId == "SESSION-001");
    assert(context.qrCheckinRepository.createdCheckins.size() == 1);
    assert(context.presenceSessionRepository.createdSessions.empty());
    assert(context.presenceSessionRepository.extendedSessionId == "SESSION-001");
    assert(context.presenceSessionRepository.extendedExpiresAt == request.expiresAt);
}

void endSessionDelegatesToRepository() {
    TestContext context{};
    auto service = makeService(context);
    const auto endedAt = testTime(120);

    service.endSession("SESSION-001", endedAt);

    assert(context.presenceSessionRepository.endedSessionId == "SESSION-001");
    assert(context.presenceSessionRepository.endedAt == endedAt);
}

}

int main() {
    rejectsMissingEmployeeWithoutCreatingCheckin();
    rejectsInactiveEmployeeWithoutCreatingCheckin();
    rejectsMissingZoneWithoutCreatingCheckin();
    createsNewPresenceSessionWhenNoActiveSessionExists();
    extendsExistingPresenceSessionWhenActiveSessionExists();
    endSessionDelegatesToRepository();
}
