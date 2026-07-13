#pragma once

#include <mongocxx/collection.hpp>

#include <chrono>
#include <optional>
#include <string>

#include "securezone/domain/CameraTrack.h"
#include "securezone/repository/ICameraTrackRepository.h"

namespace securezone::infrastructure::mongodb::repositories {

class MongoCameraTrackRepository final : public repository::ICameraTrackRepository {
public:
    explicit MongoCameraTrackRepository(mongocxx::collection cameraTracksCollection);

    std::optional<domain::CameraTrack> findByTrackId(
        const std::string& trackId
    ) const override;

    void upsert(const domain::CameraTrack& cameraTrack) override;

    void markLost(
        const std::string& trackId,
        std::chrono::system_clock::time_point lostAt
    ) override;

private:
    mongocxx::collection cameraTracksCollection_;
};

}
