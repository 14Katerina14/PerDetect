#include "securezone/api/routes/AlarmRoutes.h"

#include <cassert>
#include <chrono>
#include <optional>
#include <string>
#include <vector>

namespace {

using namespace securezone;
using namespace securezone::api;
using Clock = std::chrono::system_clock;

EndpointAuthorizationResult managerAuthorization(
    const HttpRequest&,
    auth::AuthorizationPolicy policy
) {
    assert(policy == auth::AuthorizationPolicy::ManagerOrAdmin);
    return {
        EndpointAuthorizationStatus::Authorized,
        auth::AuthenticatedPrincipal{
            "APP-MANAGER-001", "manager", domain::AppUserRole::Manager, {}
        }
    };
}

query::AlarmView alarmView(domain::AlarmStatus status) {
    query::AlarmView alarm{};
    alarm.alarmId = status == domain::AlarmStatus::Resolved
        ? "ALARM-RESOLVED" : "ALARM-ACTIVE";
    alarm.zoneId = "ZONE-001";
    alarm.zoneName = "Machine A dangerous zone";
    alarm.trackId = "CAM-001:OBJECT-42";
    alarm.employeeId = "EMP-001";
    alarm.employeeName = "Ivan Petrov";
    alarm.status = status;
    alarm.reason = "role_not_allowed";
    alarm.message = "Employee role is not allowed in this zone.";
    alarm.enteredAt = Clock::time_point{} + std::chrono::seconds{10};
    alarm.stillInside = status != domain::AlarmStatus::Resolved;
    if (status == domain::AlarmStatus::Resolved) {
        alarm.exitedAt = Clock::time_point{} + std::chrono::seconds{20};
        alarm.resolvedAt = alarm.exitedAt;
    }
    return alarm;
}

void managerCanReadActiveAndRecentAlarms() {
    AlarmRoutes routes{
        [] { return std::vector<query::AlarmView>{alarmView(domain::AlarmStatus::Active)}; },
        [] { return std::vector<query::AlarmView>{alarmView(domain::AlarmStatus::Resolved)}; },
        managerAuthorization
    };
    const HttpRequest request{"GET", "/api/alarms/active", {}, {}};

    const auto active = routes.handleActive(request);
    const auto recent = routes.handleRecent(request);

    assert(active.statusCode == 200);
    assert(active.body.find(R"("count":1)") != std::string::npos);
    assert(active.body.find(R"("alarmId":"ALARM-ACTIVE")") != std::string::npos);
    assert(active.body.find(R"("status":"active")") != std::string::npos);
    assert(active.body.find(R"("stillInside":true)") != std::string::npos);
    assert(recent.statusCode == 200);
    assert(recent.body.find(R"("alarmId":"ALARM-RESOLVED")") != std::string::npos);
    assert(recent.body.find(R"("status":"resolved")") != std::string::npos);
    assert(recent.body.find(R"("resolvedAt":"1970-01-01T00:00:20Z")") != std::string::npos);
}

void missingOrWorkerCredentialsCannotReadAlarms() {
    const auto emptyList = [] { return std::vector<query::AlarmView>{}; };
    AlarmRoutes unauthorized{
        emptyList,
        emptyList,
        [](const HttpRequest&, auth::AuthorizationPolicy) {
            return EndpointAuthorizationResult{};
        }
    };
    AlarmRoutes forbidden{
        emptyList,
        emptyList,
        [](const HttpRequest&, auth::AuthorizationPolicy) {
            return EndpointAuthorizationResult{
                EndpointAuthorizationStatus::Forbidden,
                auth::AuthenticatedPrincipal{
                    "APP-WORKER-001", "worker", domain::AppUserRole::Worker, "EMP-001"
                }
            };
        }
    };
    const HttpRequest request{"GET", "/api/alarms/active", {}, {}};

    assert(unauthorized.handleActive(request).statusCode == 401);
    assert(forbidden.handleRecent(request).statusCode == 403);
}

void missingRuntimeDependenciesReturnServiceUnavailable() {
    const HttpRequest request{"GET", "/api/alarms/active", {}, {}};
    AlarmRoutes noAuthorization{{}, {}, {}};
    AlarmRoutes noQuery{{}, {}, managerAuthorization};

    assert(noAuthorization.handleActive(request).statusCode == 503);
    assert(noQuery.handleActive(request).statusCode == 503);
}

}

int main() {
    managerCanReadActiveAndRecentAlarms();
    missingOrWorkerCredentialsCannotReadAlarms();
    missingRuntimeDependenciesReturnServiceUnavailable();
}
