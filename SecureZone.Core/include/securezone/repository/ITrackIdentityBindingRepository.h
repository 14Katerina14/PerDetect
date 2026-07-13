#pragma once

#include <optional>
#include <string>

#include "securezone/domain/TrackIdentityBinding.h"

namespace securezone::repository {

class ITrackIdentityBindingRepository {
public:
    virtual ~ITrackIdentityBindingRepository() = default;

    virtual std::optional<domain::TrackIdentityBinding> findCurrentByTrackId(
        const std::string& trackId
    ) const = 0;

    virtual void create(const domain::TrackIdentityBinding& binding) = 0;

    virtual void updateStatus(
        const std::string& bindingId,
        const std::string& status
    ) = 0;
};

}
