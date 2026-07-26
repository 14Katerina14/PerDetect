#pragma once

#include <chrono>
#include <string>

namespace securezone::domain {

enum class TrackIdentityBindingStatus { Active, Expired, Released };

struct TrackIdentityBinding {
    std::string bindingId;
    std::string cameraId;
    std::string objectId;
    std::string employeeId;
    std::string sourceCheckinId;
    std::string presenceSessionId;
    std::chrono::system_clock::time_point boundAt{};
    std::chrono::system_clock::time_point expiresAt{};
    TrackIdentityBindingStatus status{TrackIdentityBindingStatus::Active};

    bool isActiveAt(std::chrono::system_clock::time_point at) const {
        return status == TrackIdentityBindingStatus::Active
            && !cameraId.empty() && !objectId.empty() && !employeeId.empty()
            && boundAt <= at && at <= expiresAt;
    }
};

}
