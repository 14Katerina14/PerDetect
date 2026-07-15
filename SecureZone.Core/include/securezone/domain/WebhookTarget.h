#pragma once

#include <chrono>
#include <string>

namespace securezone::domain {

enum class WebhookTargetStatus {
    Active,
    Inactive
};

struct WebhookTarget {
    std::string targetId;
    std::string name;
    std::string url;
    WebhookTargetStatus status{WebhookTargetStatus::Active};
    std::chrono::system_clock::time_point createdAt{};
    std::chrono::system_clock::time_point updatedAt{};
};

}
