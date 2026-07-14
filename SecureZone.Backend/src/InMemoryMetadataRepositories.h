#pragma once

#include <chrono>
#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include "securezone/domain/CameraTrack.h"
#include "securezone/domain/MetadataEvent.h"
#include "securezone/repository/ICameraTrackRepository.h"
#include "securezone/repository/IMetadataEventRepository.h"

namespace securezone::backend {

class InMemoryCameraTrackRepository final : public repository::ICameraTrackRepository {
public:
    std::optional<domain::CameraTrack> findByTrackId(
        const std::string& trackId
    ) const override;

    void upsert(const domain::CameraTrack& cameraTrack) override;

    void markLost(
        const std::string& trackId,
        std::chrono::system_clock::time_point lostAt
    ) override;

    std::size_t size() const;

private:
    std::map<std::string, domain::CameraTrack> tracks_;
};

class InMemoryMetadataEventRepository final : public repository::IMetadataEventRepository {
public:
    void create(const domain::MetadataEvent& metadataEvent) override;

    std::vector<domain::MetadataEvent> findRecentByTrackId(
        const std::string& trackId,
        std::int64_t limit
    ) const override;

    std::size_t size() const;

private:
    std::vector<domain::MetadataEvent> events_;
};

}
