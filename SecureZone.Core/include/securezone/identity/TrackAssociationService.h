#pragma once

#include <chrono>
#include <vector>
#include "securezone/domain/Detection.h"
#include "securezone/domain/QrCheckIn.h"
#include "securezone/domain/TrackIdentityBinding.h"
#include "securezone/domain/Zone.h"
namespace securezone::identity {
class TrackAssociationService {
public:
    domain::TrackIdentityBinding associate(const domain::Detection& detection, const domain::Zone& zone,
        const std::vector<domain::QrCheckIn>& checkIns, std::chrono::system_clock::time_point now) const;
};
}
