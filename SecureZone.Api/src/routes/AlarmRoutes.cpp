#include "securezone/api/routes/AlarmRoutes.h"

#include "securezone/api/ApiResponse.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <utility>

#include <nlohmann/json.hpp>

namespace securezone::api {
namespace {

std::string alarmStatus(domain::AlarmStatus status) {
    switch (status) {
        case domain::AlarmStatus::Created: return "created";
        case domain::AlarmStatus::Active: return "active";
        case domain::AlarmStatus::Acknowledged: return "acknowledged";
        case domain::AlarmStatus::Resolved: return "resolved";
    }
    return "unknown";
}

std::string utcTimestamp(std::chrono::system_clock::time_point value) {
    const auto time = std::chrono::system_clock::to_time_t(value);
    std::tm utc{};
#ifdef _WIN32
    gmtime_s(&utc, &time);
#else
    gmtime_r(&time, &utc);
#endif
    std::ostringstream output;
    output << std::put_time(&utc, "%Y-%m-%dT%H:%M:%SZ");
    return output.str();
}

nlohmann::json alarmJson(const query::AlarmView& alarm) {
    nlohmann::json result{
        {"alarmId", alarm.alarmId},
        {"zoneId", alarm.zoneId},
        {"zoneName", alarm.zoneName},
        {"trackId", alarm.trackId},
        {"employeeId", alarm.employeeId},
        {"employeeName", alarm.employeeName},
        {"machineId", alarm.machineId},
        {"machineName", alarm.machineName},
        {"status", alarmStatus(alarm.status)},
        {"reason", alarm.reason},
        {"message", alarm.message},
        {"enteredAt", utcTimestamp(alarm.enteredAt)},
        {"stillInside", alarm.stillInside}
    };
    if (alarm.resolvedAt.has_value()) {
        result["resolvedAt"] = utcTimestamp(*alarm.resolvedAt);
        result["exitedAt"] = utcTimestamp(alarm.exitedAt);
    } else {
        result["resolvedAt"] = nullptr;
        result["exitedAt"] = nullptr;
    }
    return result;
}

}

AlarmRoutes::AlarmRoutes(
    ListHandler activeHandler,
    ListHandler recentHandler,
    EndpointAuthorizer::Handler authorizeHandler
) : activeHandler_{std::move(activeHandler)},
    recentHandler_{std::move(recentHandler)},
    authorizeHandler_{std::move(authorizeHandler)} {
}

HttpResponse AlarmRoutes::handleActive(const HttpRequest& request) const {
    return handleList(request, activeHandler_);
}

HttpResponse AlarmRoutes::handleRecent(const HttpRequest& request) const {
    return handleList(request, recentHandler_);
}

HttpResponse AlarmRoutes::handleList(
    const HttpRequest& request,
    const ListHandler& handler
) const {
    if (!authorizeHandler_) {
        return jsonResponse(503, R"({"error":"authorization_unavailable"})");
    }

    const auto authorization = authorizeHandler_(
        request,
        auth::AuthorizationPolicy::ManagerOrAdmin
    );
    if (authorization.status == EndpointAuthorizationStatus::Unauthorized) {
        return jsonResponse(401, R"({"error":"unauthorized"})");
    }
    if (authorization.status == EndpointAuthorizationStatus::Forbidden) {
        return jsonResponse(403, R"({"error":"forbidden"})");
    }
    if (!handler) {
        return jsonResponse(503, R"({"error":"service_unavailable"})");
    }

    nlohmann::json alarms = nlohmann::json::array();
    for (const auto& alarm : handler()) {
        alarms.push_back(alarmJson(alarm));
    }
    return jsonOk(nlohmann::json{
        {"count", alarms.size()},
        {"alarms", std::move(alarms)}
    }.dump());
}

}
