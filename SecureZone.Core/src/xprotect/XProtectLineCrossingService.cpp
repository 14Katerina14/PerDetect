#include "securezone/xprotect/XProtectLineCrossingService.h"

#include <utility>

namespace securezone::xprotect {
namespace {

bool hasRequiredFields(const XProtectLineCrossingCommand& command) {
    return !command.eventName.empty() && !command.sourceName.empty();
}

XProtectLineCrossingDecision invalidRequest(std::string message) {
    return XProtectLineCrossingDecision{
        false,
        "invalid_request",
        "none",
        {},
        {},
        {},
        std::move(message)
    };
}

XProtectLineCrossingDecision zoneNotFound() {
    return XProtectLineCrossingDecision{
        false,
        "zone_not_found",
        "none",
        {},
        {},
        {},
        "No SecureZone zone is mapped to the XProtect line crossing event."
    };
}

XProtectLineCrossingDecision violation(
    const domain::Zone& zone,
    std::string message
) {
    return XProtectLineCrossingDecision{
        true,
        "processed",
        "violation",
        zone.zoneId,
        {},
        {},
        std::move(message)
    };
}

XProtectLineCrossingDecision allowed(
    const domain::Zone& zone,
    const domain::PresenceSession& session
) {
    return XProtectLineCrossingDecision{
        true,
        "processed",
        "allowed",
        zone.zoneId,
        session.sessionId,
        session.employeeId,
        "Active QR presence session found for zone."
    };
}

XProtectLineCrossingDecision allowed(
    const domain::Zone& zone,
    const domain::TrackIdentityBinding& binding
) {
    return XProtectLineCrossingDecision{
        true, "processed", "allowed", zone.zoneId,
        binding.presenceSessionId, binding.employeeId,
        "Active QR identity binding found for camera object."
    };
}

}

XProtectLineCrossingService::XProtectLineCrossingService(
    ZoneResolver zoneResolver,
    ActivePresenceResolver activePresenceResolver
) : zoneResolver_{std::move(zoneResolver)},
    activePresenceResolver_{std::move(activePresenceResolver)} {
}

XProtectLineCrossingService::XProtectLineCrossingService(
    ZoneResolver zoneResolver,
    IdentityBindingResolver identityBindingResolver
) : zoneResolver_{std::move(zoneResolver)},
    identityBindingResolver_{std::move(identityBindingResolver)} {
}

XProtectLineCrossingService::XProtectLineCrossingService(
    ZoneResolver zoneResolver,
    IdentityBindingResolver identityBindingResolver,
    PolicyAlarmEvaluator policyAlarmEvaluator
) : zoneResolver_{std::move(zoneResolver)},
    identityBindingResolver_{std::move(identityBindingResolver)},
    policyAlarmEvaluator_{std::move(policyAlarmEvaluator)} {
}

XProtectLineCrossingDecision XProtectLineCrossingService::evaluate(
    const XProtectLineCrossingCommand& command
) const {
    if (!hasRequiredFields(command)) {
        return invalidRequest("eventName and sourceName are required.");
    }

    if (!zoneResolver_ || (!activePresenceResolver_ && !identityBindingResolver_)) {
        return XProtectLineCrossingDecision{
            false,
            "handler_error",
            "none",
            {},
            {},
            {},
            "XProtect line crossing service dependencies are not configured."
        };
    }

    const auto zone = zoneResolver_(command);
    if (!zone.has_value() || zone->status != domain::ZoneStatus::Active) {
        return zoneNotFound();
    }

    if (identityBindingResolver_) {
        std::optional<domain::TrackIdentityBinding> binding;
        if (!command.cameraId.empty() && !command.objectId.empty()) {
            binding = identityBindingResolver_(
                command.cameraId, command.objectId, command.receivedAt
            );
        }

        if (policyAlarmEvaluator_) {
            return policyAlarmEvaluator_(command, *zone, binding);
        }

        if (command.cameraId.empty() || command.objectId.empty()) {
            return violation(*zone, "Line crossing event has no cameraId or ObjectId identity evidence.");
        }

        if (!binding.has_value() || !binding->isActiveAt(command.receivedAt)) {
            return violation(*zone, "Camera object has no active QR identity binding.");
        }
        return allowed(*zone, *binding);
    }

    const auto activePresence = activePresenceResolver_(*zone, command.receivedAt);
    if (!activePresence.has_value()) {
        return violation(*zone, "No active QR presence session found for zone.");
    }

    if (!activePresence->isActiveAt(command.receivedAt)) {
        return violation(*zone, "QR presence session is not active at event time.");
    }

    return allowed(*zone, *activePresence);
}

}
