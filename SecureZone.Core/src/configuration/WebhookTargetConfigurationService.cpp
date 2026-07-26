#include "securezone/configuration/WebhookTargetConfigurationService.h"

#include <utility>

namespace securezone::configuration {

namespace {

bool isValid(const domain::WebhookTarget& target) {
    constexpr const char* HttpsPrefix = "https://";
    return !target.targetId.empty()
        && !target.name.empty()
        && target.url.rfind(HttpsPrefix, 0) == 0
        && target.url.size() > std::char_traits<char>::length(HttpsPrefix);
}

}

WebhookTargetConfigurationService::WebhookTargetConfigurationService(
    repository::IWebhookTargetRepository& targetRepository
) : targetRepository_{targetRepository} {
}

WebhookTargetConfigurationResult WebhookTargetConfigurationService::add(
    domain::WebhookTarget target,
    std::chrono::system_clock::time_point now
) const {
    if (!isValid(target)) {
        return {WebhookTargetConfigurationStatus::InvalidTarget};
    }

    if (targetRepository_.findByTargetId(target.targetId)) {
        return {WebhookTargetConfigurationStatus::TargetAlreadyExists};
    }

    target.status = domain::WebhookTargetStatus::Active;
    target.createdAt = now;
    target.updatedAt = now;
    return {
        targetRepository_.save(target)
            ? WebhookTargetConfigurationStatus::Updated
            : WebhookTargetConfigurationStatus::RepositoryFailure
    };
}

WebhookTargetConfigurationResult WebhookTargetConfigurationService::enable(
    const std::string& targetId,
    std::chrono::system_clock::time_point now
) const {
    return setStatus(targetId, domain::WebhookTargetStatus::Active, now);
}

WebhookTargetConfigurationResult WebhookTargetConfigurationService::disable(
    const std::string& targetId,
    std::chrono::system_clock::time_point now
) const {
    return setStatus(targetId, domain::WebhookTargetStatus::Inactive, now);
}

std::vector<domain::WebhookTarget> WebhookTargetConfigurationService::listActive() const {
    return targetRepository_.findActive();
}

WebhookTargetConfigurationResult WebhookTargetConfigurationService::setStatus(
    const std::string& targetId,
    domain::WebhookTargetStatus status,
    std::chrono::system_clock::time_point now
) const {
    if (targetId.empty()) {
        return {WebhookTargetConfigurationStatus::InvalidTarget};
    }

    auto target = targetRepository_.findByTargetId(targetId);
    if (!target) {
        return {WebhookTargetConfigurationStatus::TargetNotFound};
    }

    target->status = status;
    target->updatedAt = now;
    return {
        targetRepository_.save(*target)
            ? WebhookTargetConfigurationStatus::Updated
            : WebhookTargetConfigurationStatus::RepositoryFailure
    };
}

}
