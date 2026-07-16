#pragma once

#include "securezone/domain/PushNotification.h"

#include <string>

namespace securezone::notification {

struct PushSendResult {
    bool delivered{};
    int responseCode{};
    std::string error;
};

class IPushNotificationProvider {
public:
    virtual ~IPushNotificationProvider() = default;
    virtual PushSendResult send(
        const domain::PushSubscription& subscription,
        const domain::PushMessage& message
    ) = 0;
};

}
