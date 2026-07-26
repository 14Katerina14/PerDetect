#pragma once

#include "securezone/presence/PresenceSessionService.h"
#include "securezone/identity/CameraIdentityService.h"

#include <chrono>
#include <functional>
#include <string>
#include <string_view>

namespace securezone::qr {

struct QrCheckInCommand {
    std::string employeeId;
    std::string zoneId;
    std::string scannedByUserId;
    std::string cameraId;
};

struct QrCheckInResult {
    bool accepted{};
    std::string status;
    std::string sessionId;
    std::string message;
    std::string objectId;
    std::string bindingId;
};

std::string_view toQrCheckInStatus(
    presence::PresenceSessionStartStatus status
);

std::string_view toQrCheckInMessage(
    presence::PresenceSessionStartStatus status
);

class QrCheckInService {
public:
    using Clock = std::chrono::system_clock;
    using NowProvider = std::function<Clock::time_point()>;

    explicit QrCheckInService(
        presence::PresenceSessionService& presenceSessionService,
        NowProvider nowProvider = [] { return Clock::now(); },
        std::chrono::minutes presenceDuration = std::chrono::minutes{2}
    );

    QrCheckInService(
        presence::PresenceSessionService& presenceSessionService,
        identity::CameraIdentityService& cameraIdentityService,
        NowProvider nowProvider = [] { return Clock::now(); },
        std::chrono::minutes presenceDuration = std::chrono::minutes{2}
    );

    QrCheckInResult checkIn(const QrCheckInCommand& command);

private:
    presence::PresenceSessionService& presenceSessionService_;
    NowProvider nowProvider_;
    std::chrono::minutes presenceDuration_;
    identity::CameraIdentityService* cameraIdentityService_{};
};

}
