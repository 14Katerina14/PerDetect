#pragma once

#include <chrono>
#include <functional>
#include <string>
#include <string_view>

#include "securezone/presence/PresenceSessionService.h"

namespace securezone::api::routes {

struct QrCheckInRequest {
    std::string employeeId;
    std::string zoneId;
    std::string scannedByUserId;
};

struct QrCheckInResponse {
    int statusCode{200};
    bool accepted{};
    std::string status;
    std::string sessionId;
    std::string message;
};

std::string_view toQrApiStatus(
    presence::PresenceSessionStartStatus status
);

std::string_view toQrApiMessage(
    presence::PresenceSessionStartStatus status
);

class QrRoutes {
public:
    using Clock = std::chrono::system_clock;
    using NowProvider = std::function<Clock::time_point()>;

    static constexpr std::string_view CheckInMethod{"POST"};
    static constexpr std::string_view CheckInPath{"/api/qr/check-in"};

    explicit QrRoutes(
        presence::PresenceSessionService& presenceSessionService,
        NowProvider nowProvider = [] { return Clock::now(); }
    );

    QrCheckInResponse handleCheckIn(const QrCheckInRequest& request);

private:
    presence::PresenceSessionService& presenceSessionService_;
    NowProvider nowProvider_;
};

}
