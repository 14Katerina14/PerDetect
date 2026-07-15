#include "securezone/xprotect/XProtectLineCrossingService.h"

#include <cassert>
#include <chrono>
#include <optional>
#include <string>

namespace {

using namespace securezone;
using Clock = std::chrono::system_clock;

const auto EventTime = Clock::time_point{} + std::chrono::minutes{10};

domain::Zone activeZone() {
    domain::Zone zone{};
    zone.zoneId = "ZONE-001";
    zone.name = "Machine A Dangerous Zone";
    zone.cameraId = "CAM-001";
    zone.type = domain::ZoneType::Dangerous;
    zone.status = domain::ZoneStatus::Active;
    zone.xprotectEventName = "Channel.<int>.OpenSDK.WiseAI.LineCrossing.<int>.State-2";
    return zone;
}

domain::PresenceSession activePresenceSession() {
    domain::PresenceSession session{};
    session.sessionId = "SESSION-001";
    session.employeeId = "EMP-001";
    session.zoneId = "ZONE-001";
    session.sourceCheckinId = "CHECKIN-001";
    session.startedAt = EventTime - std::chrono::minutes{1};
    session.expiresAt = EventTime + std::chrono::minutes{1};
    session.status = domain::PresenceSessionStatus::Active;
    return session;
}

domain::TrackIdentityBinding activeIdentityBinding() {
    domain::TrackIdentityBinding binding{};
    binding.bindingId = "BINDING-001";
    binding.cameraId = "CAM-001";
    binding.objectId = "OBJECT-42";
    binding.employeeId = "EMP-001";
    binding.presenceSessionId = "SESSION-001";
    binding.boundAt = EventTime - std::chrono::minutes{1};
    binding.expiresAt = EventTime + std::chrono::minutes{1};
    binding.status = domain::TrackIdentityBindingStatus::Active;
    return binding;
}

xprotect::XProtectLineCrossingCommand validCommand() {
    return {
        "Channel.<int>.OpenSDK.WiseAI.LineCrossing.<int>.State-2",
        "Hanwha Vision TNO-C4052T TEST-CAMERA - Camera 1",
        EventTime
    };
}

xprotect::XProtectLineCrossingService makeService(
    std::optional<domain::Zone> zone,
    std::optional<domain::PresenceSession> session
) {
    return xprotect::XProtectLineCrossingService{
        [zone](const xprotect::XProtectLineCrossingCommand&) {
            return zone;
        },
        [session](const domain::Zone&, Clock::time_point) {
            return session;
        }
    };
}

void rejectsMissingRequiredFields() {
    auto service = makeService(activeZone(), activePresenceSession());

    auto command = validCommand();
    command.eventName.clear();

    const auto result = service.evaluate(command);

    assert(!result.accepted);
    assert(result.status == "invalid_request");
    assert(result.decision == "none");
    assert(result.message == "eventName and sourceName are required.");
}

void returnsZoneNotFoundWhenEventCannotBeMapped() {
    auto service = makeService(std::nullopt, std::nullopt);

    const auto result = service.evaluate(validCommand());

    assert(!result.accepted);
    assert(result.status == "zone_not_found");
    assert(result.decision == "none");
    assert(result.zoneId.empty());
}

void returnsZoneNotFoundForInactiveZone() {
    auto zone = activeZone();
    zone.status = domain::ZoneStatus::Inactive;
    auto service = makeService(zone, activePresenceSession());

    const auto result = service.evaluate(validCommand());

    assert(!result.accepted);
    assert(result.status == "zone_not_found");
    assert(result.decision == "none");
}

void createsViolationWhenNoPresenceSessionExists() {
    auto service = makeService(activeZone(), std::nullopt);

    const auto result = service.evaluate(validCommand());

    assert(result.accepted);
    assert(result.status == "processed");
    assert(result.decision == "violation");
    assert(result.zoneId == "ZONE-001");
    assert(result.sessionId.empty());
    assert(result.employeeId.empty());
    assert(result.message == "No active QR presence session found for zone.");
}

void createsViolationWhenPresenceSessionIsExpired() {
    auto session = activePresenceSession();
    session.expiresAt = EventTime - std::chrono::seconds{1};
    auto service = makeService(activeZone(), session);

    const auto result = service.evaluate(validCommand());

    assert(result.accepted);
    assert(result.status == "processed");
    assert(result.decision == "violation");
    assert(result.zoneId == "ZONE-001");
    assert(result.message == "QR presence session is not active at event time.");
}

void allowsWhenActivePresenceSessionExists() {
    auto service = makeService(activeZone(), activePresenceSession());

    const auto result = service.evaluate(validCommand());

    assert(result.accepted);
    assert(result.status == "processed");
    assert(result.decision == "allowed");
    assert(result.zoneId == "ZONE-001");
    assert(result.sessionId == "SESSION-001");
    assert(result.employeeId == "EMP-001");
    assert(result.message == "Active QR presence session found for zone.");
}

void reportsHandlerErrorWhenDependenciesAreMissing() {
    xprotect::XProtectLineCrossingService service{
        xprotect::XProtectLineCrossingService::ZoneResolver{},
        xprotect::XProtectLineCrossingService::ActivePresenceResolver{}
    };

    const auto result = service.evaluate(validCommand());

    assert(!result.accepted);
    assert(result.status == "handler_error");
    assert(result.decision == "none");
}

void identityModeRequiresCameraObjectEvidence() {
    xprotect::XProtectLineCrossingService service{
        [](const xprotect::XProtectLineCrossingCommand&) {
            return std::optional<domain::Zone>{activeZone()};
        },
        xprotect::XProtectLineCrossingService::IdentityBindingResolver{
            [](const std::string&, const std::string&, Clock::time_point) {
                return std::optional<domain::TrackIdentityBinding>{activeIdentityBinding()};
            }
        }
    };

    const auto result = service.evaluate(validCommand());

    assert(result.accepted);
    assert(result.decision == "violation");
    assert(result.message.find("ObjectId") != std::string::npos);
}

void identityModeAllowsOnlyTheBoundCameraObject() {
    xprotect::XProtectLineCrossingService service{
        [](const xprotect::XProtectLineCrossingCommand&) {
            return std::optional<domain::Zone>{activeZone()};
        },
        xprotect::XProtectLineCrossingService::IdentityBindingResolver{
            [](const std::string& cameraId, const std::string& objectId, Clock::time_point) {
                if (cameraId == "CAM-001" && objectId == "OBJECT-42") {
                    return std::optional<domain::TrackIdentityBinding>{activeIdentityBinding()};
                }
                return std::optional<domain::TrackIdentityBinding>{};
            }
        }
    };
    auto command = validCommand();
    command.cameraId = "CAM-001";
    command.objectId = "OBJECT-42";

    const auto allowed = service.evaluate(command);
    command.objectId = "OBJECT-99";
    const auto violation = service.evaluate(command);

    assert(allowed.accepted && allowed.decision == "allowed");
    assert(allowed.employeeId == "EMP-001");
    assert(allowed.sessionId == "SESSION-001");
    assert(violation.accepted && violation.decision == "violation");
    assert(violation.employeeId.empty());
}

}

int main() {
    rejectsMissingRequiredFields();
    returnsZoneNotFoundWhenEventCannotBeMapped();
    returnsZoneNotFoundForInactiveZone();
    createsViolationWhenNoPresenceSessionExists();
    createsViolationWhenPresenceSessionIsExpired();
    allowsWhenActivePresenceSessionExists();
    reportsHandlerErrorWhenDependenciesAreMissing();
    identityModeRequiresCameraObjectEvidence();
    identityModeAllowsOnlyTheBoundCameraObject();
}
