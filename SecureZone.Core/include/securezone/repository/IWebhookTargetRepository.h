#pragma once
#include <chrono>
#include <optional>
#include <string>
#include <vector>
namespace securezone::repository { enum class WebhookTargetStatus { Active, Inactive }; struct WebhookTarget { std::string targetId,name,url; WebhookTargetStatus status{WebhookTargetStatus::Active}; std::chrono::system_clock::time_point createdAt{},updatedAt{}; }; class IWebhookTargetRepository { public: virtual ~IWebhookTargetRepository()=default; virtual std::optional<WebhookTarget> findByTargetId(const std::string&)const=0; virtual std::vector<WebhookTarget> findActive()const=0; virtual bool save(const WebhookTarget&)=0; }; }
