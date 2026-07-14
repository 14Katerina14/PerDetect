#pragma once

#include <chrono>
#include <string>
#include <vector>

#include "securezone/domain/WebhookTarget.h"
#include "securezone/repository/IWebhookTargetRepository.h"

namespace securezone::configuration {

enum class WebhookTargetConfigurationStatus {
    Updated,
    InvalidTarget,
    TargetNotFound,
    TargetAlreadyExists,
    RepositoryFailure
};

struct WebhookTargetConfigurationResult {
    WebhookTargetConfigurationStatus status{};

    bool succeeded() const {
        return status == WebhookTargetConfigurationStatus::Updated;
    }
};

class WebhookTargetConfigurationService {
public:
    explicit WebhookTargetConfigurationService(
        repository::IWebhookTargetRepository& targetRepository
    );

    WebhookTargetConfigurationResult add(
        domain::WebhookTarget target,
        std::chrono::system_clock::time_point now =
            std::chrono::system_clock::now()
    ) const;
    WebhookTargetConfigurationResult enable(
        const std::string& targetId,
        std::chrono::system_clock::time_point now =
            std::chrono::system_clock::now()
    ) const;
    WebhookTargetConfigurationResult disable(
        const std::string& targetId,
        std::chrono::system_clock::time_point now =
            std::chrono::system_clock::now()
    ) const;
    std::vector<domain::WebhookTarget> listActive() const;

private:
    WebhookTargetConfigurationResult setStatus(
        const std::string& targetId,
        domain::WebhookTargetStatus status,
        std::chrono::system_clock::time_point now
    ) const;

    repository::IWebhookTargetRepository& targetRepository_;
};

}
