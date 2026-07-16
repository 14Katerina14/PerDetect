#include "securezone/query/ActivePresenceQueryService.h"

#include <algorithm>
#include <utility>

namespace securezone::query {
namespace {

std::size_t normalizedLimit(std::size_t limit) {
    return std::min<std::size_t>(limit == 0U ? 50U : limit, 100U);
}

}

ActivePresenceQueryService::ActivePresenceQueryService(
    repository::IPresenceSessionReadRepository& presenceRepository,
    repository::IEmployeeRepository& employeeRepository,
    repository::IZoneRepository& zoneRepository
) : presenceRepository_{presenceRepository},
    employeeRepository_{employeeRepository},
    zoneRepository_{zoneRepository} {
}

std::vector<ActivePresenceView> ActivePresenceQueryService::list(
    std::chrono::system_clock::time_point at,
    std::size_t limit,
    const std::optional<std::string>& zoneId
) const {
    std::vector<ActivePresenceView> views;
    for (const auto& session : presenceRepository_.findActiveAt(
             at,
             normalizedLimit(limit),
             zoneId
         )) {
        ActivePresenceView view{};
        view.sessionId = session.sessionId;
        view.employeeId = session.employeeId;
        view.zoneId = session.zoneId;
        view.startedAt = session.startedAt;
        view.expiresAt = session.expiresAt;

        const auto employee = employeeRepository_.findByEmployeeId(session.employeeId);
        if (employee.has_value()) view.employeeName = employee->fullName;
        const auto zone = zoneRepository_.findByZoneId(session.zoneId);
        if (zone.has_value()) view.zoneName = zone->name;
        views.push_back(std::move(view));
    }
    return views;
}

}
