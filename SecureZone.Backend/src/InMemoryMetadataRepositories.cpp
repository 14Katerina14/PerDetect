#include "InMemoryMetadataRepositories.h"

#include <algorithm>
#include <optional>
#include <string>
#include <vector>

namespace securezone::backend {

std::optional<domain::CameraTrack> InMemoryCameraTrackRepository::findByTrackId(
    const std::string& trackId
) const {
    const auto iterator = tracks_.find(trackId);
    if (iterator == tracks_.end()) {
        return std::nullopt;
    }

    return iterator->second;
}

void InMemoryCameraTrackRepository::upsert(const domain::CameraTrack& cameraTrack) {
    tracks_[cameraTrack.trackId] = cameraTrack;
}

void InMemoryCameraTrackRepository::markLost(
    const std::string& trackId,
    std::chrono::system_clock::time_point lostAt
) {
    auto iterator = tracks_.find(trackId);
    if (iterator == tracks_.end()) {
        return;
    }

    iterator->second.status = "lost";
    iterator->second.lastSeenAt = lostAt;
}

std::size_t InMemoryCameraTrackRepository::size() const {
    return tracks_.size();
}

void InMemoryMetadataEventRepository::create(
    const domain::MetadataEvent& metadataEvent
) {
    events_.push_back(metadataEvent);
}

std::vector<domain::MetadataEvent> InMemoryMetadataEventRepository::findRecentByTrackId(
    const std::string& trackId,
    std::int64_t limit
) const {
    std::vector<domain::MetadataEvent> result;

    for (auto iterator = events_.rbegin(); iterator != events_.rend(); ++iterator) {
        if (iterator->trackId != trackId) {
            continue;
        }

        result.push_back(*iterator);
        if (static_cast<std::int64_t>(result.size()) >= limit) {
            break;
        }
    }

    return result;
}

std::size_t InMemoryMetadataEventRepository::size() const {
    return events_.size();
}

}
