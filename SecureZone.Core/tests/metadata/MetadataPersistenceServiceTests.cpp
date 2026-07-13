#include "securezone/metadata/MetadataPersistenceService.h"

#include <cassert>
#include <chrono>
#include <optional>
#include <string>
#include <vector>

namespace {

using Clock = std::chrono::system_clock;

class FakeCameraTrackRepository final : public securezone::repository::ICameraTrackRepository {
public:
    std::optional<securezone::domain::CameraTrack> existingTrack;
    std::vector<securezone::domain::CameraTrack> upsertedTracks;

    std::optional<securezone::domain::CameraTrack> findByTrackId(
        const std::string& trackId
    ) const override {
        if (existingTrack.has_value() && existingTrack->trackId == trackId) {
            return existingTrack;
        }

        return std::nullopt;
    }

    void upsert(const securezone::domain::CameraTrack& cameraTrack) override {
        upsertedTracks.push_back(cameraTrack);
    }

    void markLost(
        const std::string&,
        Clock::time_point
    ) override {
    }
};

class FakeMetadataEventRepository final : public securezone::repository::IMetadataEventRepository {
public:
    std::vector<securezone::domain::MetadataEvent> createdEvents;

    void create(const securezone::domain::MetadataEvent& metadataEvent) override {
        createdEvents.push_back(metadataEvent);
    }

    std::vector<securezone::domain::MetadataEvent> findRecentByTrackId(
        const std::string&,
        std::int64_t
    ) const override {
        return {};
    }
};

securezone::domain::CameraTrack createTrack(
    const std::string& trackId,
    Clock::time_point firstSeenAt,
    Clock::time_point lastSeenAt
) {
    securezone::domain::CameraTrack track{};
    track.trackId = trackId;
    track.cameraId = "CAM-001";
    track.firstSeenAt = firstSeenAt;
    track.lastSeenAt = lastSeenAt;
    track.currentZoneId = "ZONE-001";
    track.objectClass = "Person";
    track.bbox = {10.0, 20.0, 100.0, 200.0};
    track.status = "active";
    return track;
}

securezone::domain::MetadataEvent createEvent(const std::string& eventId) {
    securezone::domain::MetadataEvent event{};
    event.eventId = eventId;
    event.cameraId = "CAM-001";
    event.trackId = "TRACK-001";
    event.timestamp = Clock::time_point{std::chrono::seconds{30}};
    event.objectClass = "Person";
    event.bbox = {10.0, 20.0, 100.0, 200.0};
    event.zoneId = "ZONE-001";
    event.eventType = "DetectionUpdate";
    return event;
}

void persistsTracksAndMetadataEvents() {
    FakeCameraTrackRepository cameraTrackRepository{};
    FakeMetadataEventRepository metadataEventRepository{};
    securezone::metadata::MetadataPersistenceService service{
        cameraTrackRepository,
        metadataEventRepository
    };

    securezone::metadata::MetadataIngestionResult ingestionResult{};
    ingestionResult.cameraTracks.push_back(createTrack(
        "TRACK-001",
        Clock::time_point{std::chrono::seconds{10}},
        Clock::time_point{std::chrono::seconds{20}}
    ));
    ingestionResult.metadataEvents.push_back(createEvent("EVENT-001"));
    ingestionResult.metadataEvents.push_back(createEvent("EVENT-002"));

    const auto result = service.persist(ingestionResult);

    assert(result.tracksUpserted == 1);
    assert(result.eventsCreated == 2);
    assert(cameraTrackRepository.upsertedTracks.size() == 1);
    assert(metadataEventRepository.createdEvents.size() == 2);
    assert(cameraTrackRepository.upsertedTracks.front().trackId == "TRACK-001");
    assert(metadataEventRepository.createdEvents.front().eventId == "EVENT-001");
}

void preservesExistingTrackFirstSeenAtAndZone() {
    FakeCameraTrackRepository cameraTrackRepository{};
    cameraTrackRepository.existingTrack = createTrack(
        "TRACK-001",
        Clock::time_point{std::chrono::seconds{5}},
        Clock::time_point{std::chrono::seconds{8}}
    );
    cameraTrackRepository.existingTrack->currentZoneId = "ZONE-OLD";

    FakeMetadataEventRepository metadataEventRepository{};
    securezone::metadata::MetadataPersistenceService service{
        cameraTrackRepository,
        metadataEventRepository
    };

    securezone::metadata::MetadataIngestionResult ingestionResult{};
    auto incomingTrack = createTrack(
        "TRACK-001",
        Clock::time_point{std::chrono::seconds{10}},
        Clock::time_point{std::chrono::seconds{20}}
    );
    incomingTrack.currentZoneId = {};
    ingestionResult.cameraTracks.push_back(incomingTrack);

    const auto result = service.persist(ingestionResult);

    assert(result.tracksUpserted == 1);
    assert(cameraTrackRepository.upsertedTracks.size() == 1);

    const auto& persistedTrack = cameraTrackRepository.upsertedTracks.front();
    assert(persistedTrack.firstSeenAt == Clock::time_point{std::chrono::seconds{5}});
    assert(persistedTrack.lastSeenAt == Clock::time_point{std::chrono::seconds{20}});
    assert(persistedTrack.currentZoneId == "ZONE-OLD");
}

}

int main() {
    persistsTracksAndMetadataEvents();
    preservesExistingTrackFirstSeenAtAndZone();
}
