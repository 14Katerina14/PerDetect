#pragma once

#include <mongocxx/collection.hpp>

#include <optional>
#include <string>

#include "securezone/domain/TrackIdentityBinding.h"
#include "securezone/repository/ITrackIdentityBindingRepository.h"

namespace securezone::infrastructure::mongodb::repositories {

class MongoTrackIdentityBindingRepository final : public repository::ITrackIdentityBindingRepository {
public:
    explicit MongoTrackIdentityBindingRepository(mongocxx::collection trackIdentityBindingsCollection);

    std::optional<domain::TrackIdentityBinding> findCurrentByTrackId(
        const std::string& trackId
    ) const override;

    void create(const domain::TrackIdentityBinding& binding) override;

    void updateStatus(
        const std::string& bindingId,
        const std::string& status
    ) override;

private:
    mongocxx::collection trackIdentityBindingsCollection_;
};

}
