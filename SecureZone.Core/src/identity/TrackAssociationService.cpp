#include "securezone/identity/TrackAssociationService.h"
namespace securezone::identity {
domain::TrackIdentityBinding TrackAssociationService::associate(const domain::Detection& detection, const domain::Zone& zone, const std::vector<domain::QrCheckIn>& checkIns, std::chrono::system_clock::time_point now) const {
    std::vector<const domain::QrCheckIn*> matches;
    for (const auto& checkIn : checkIns) if (checkIn.zoneId == zone.zoneId && checkIn.status == domain::QrCheckInStatus::Active && checkIn.scannedAt <= now && now <= checkIn.validUntil) matches.push_back(&checkIn);
    if (matches.size() != 1) return {detection.trackId, {}, {}, matches.empty() ? domain::BindingStatus::Expired : domain::BindingStatus::Uncertain, 0.0, domain::BindingSource::QrCheckIn, now};
    const auto& match = *matches.front();
    return {detection.trackId, match.employeeId, match.checkInId, domain::BindingStatus::Bound, 1.0, domain::BindingSource::QrCheckIn, now};
}
}
