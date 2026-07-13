#include "securezone/identity/TrackAssociationService.h"

#include <chrono>
#include <string>
#include <vector>

namespace securezone::identity {

namespace {

std::string createBindingId(
    const std::string& trackId,
    std::chrono::system_clock::time_point boundAt
) {
    const auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(
        boundAt.time_since_epoch()
    ).count();

    return "BIND-" + trackId + "-" + std::to_string(milliseconds);
}

domain::TrackIdentityBinding makeBinding(
    const domain::Detection& detection,
    std::chrono::system_clock::time_point boundAt
) {
    domain::TrackIdentityBinding binding{};
    binding.bindingId = createBindingId(detection.trackId, boundAt);
    binding.trackId = detection.trackId;
    binding.source = domain::BindingSource::QrCheckIn;
    binding.boundAt = boundAt;
    return binding;
}

}

domain::TrackIdentityBinding TrackAssociationService::associate(
    const domain::Detection& detection,
    const domain::Zone& zone,
    const std::vector<domain::QrCheckIn>& checkIns,
    std::chrono::system_clock::time_point now
) const {
    std::vector<const domain::QrCheckIn*> matches;
    for (const auto& checkIn : checkIns) {
        if (checkIn.zoneId == zone.zoneId && checkIn.isActiveAt(now)) {
            matches.push_back(&checkIn);
        }
    }

    auto binding = makeBinding(detection, now);
    if (matches.size() != 1) {
        binding.status = matches.empty()
            ? domain::BindingStatus::Expired
            : domain::BindingStatus::Uncertain;
        return binding;
    }

    const auto& match = *matches.front();
    binding.employeeId = match.employeeId;
    binding.checkInId = match.checkInId;
    binding.status = domain::BindingStatus::Bound;
    binding.confidence = 1.0;
    return binding;
}

}
