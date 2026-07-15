#pragma once

#include "securezone/api/http/HttpRequest.h"
#include "securezone/api/http/HttpResponse.h"
#include "securezone/qr/QrCheckInService.h"

#include <functional>

namespace securezone::api {

class QrRoutes {
public:
    using CheckInHandler = std::function<qr::QrCheckInResult(const qr::QrCheckInCommand&)>;

    QrRoutes() = default;
    explicit QrRoutes(CheckInHandler checkInHandler);

    HttpResponse handleCheckIn(const HttpRequest& request) const;

private:
    CheckInHandler checkInHandler_;
};

}
