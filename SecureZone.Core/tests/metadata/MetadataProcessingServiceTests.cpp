#include "securezone/metadata/MetadataProcessingService.h"

#include <cassert>
#include <chrono>
#include <optional>
#include <string>
#include <vector>

namespace {

using Clock = std::chrono::system_clock;

class FakeMetadataParser final : public securezone::metadata::IMetadataParser {
public:
    securezone::metadata::ParsedMetadataFrame parse(
        const std::string& cameraId,
        const std::string& rawMetadata
    ) const override {
        assert(cameraId == "CAM-001");
        assert(rawMetadata == "<metadata/>");

        const auto timestamp = Clock::time_point{std::chrono::seconds{30}};

        securezone::domain::Detection detection{};
        detection.trackId = "TRACK-001";
        detection.cameraId = cameraId;
        detection.objectClass = securezone::domain::ObjectClass::Person;
        detection.bbox = {10.0, 20.0, 100.0, 200.0};
        detection.confidence = 0.92;
        detection.timestamp = timestamp;

        securezone::domain::MetadataEvent metadataEvent{};
        metadataEvent.eventId = "EVENT-001";
        metadataEvent.cameraId = cameraId;
        metadataEvent.trackId = "TRACK-001";
        metadataEvent.timestamp = timestamp;
        metadataEvent.objectClass = "Person";
        metadataEvent.bbox = detection.bbox;
        metadataEvent.eventType = "ObjectDetection";

        return securezone::metadata::ParsedMetadataFrame{
            cameraId,
            "WiseAI",
            timestamp,
            {detection},
            {metadataEvent}
        };
    }
};

class FakeCameraTrackRepository final : public securezone::repository::ICameraTrackRepository {
public:
    std::vector<securezone::domain::CameraTrack> upsertedTracks;

    std::optional<securezone::domain::CameraTrack> findByTrackId(
        const std::string&
    ) const override {
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

void processesRawMetadataThroughIngestionAndPersistence() {
    const FakeMetadataParser parser{};
    const securezone::metadata::MetadataIngestionService ingestionService{parser};

    FakeCameraTrackRepository cameraTrackRepository{};
    FakeMetadataEventRepository metadataEventRepository{};
    securezone::metadata::MetadataPersistenceService persistenceService{
        cameraTrackRepository,
        metadataEventRepository
    };

    securezone::metadata::MetadataProcessingService processingService{
        ingestionService,
        persistenceService
    };

    const auto result = processingService.process("CAM-001", "<metadata/>");

    assert(result.detectionsProcessed == 1);
    assert(result.tracksUpserted == 1);
    assert(result.eventsCreated == 2);

    assert(cameraTrackRepository.upsertedTracks.size() == 1);
    assert(cameraTrackRepository.upsertedTracks.front().trackId == "TRACK-001");
    assert(metadataEventRepository.createdEvents.size() == 2);
    assert(metadataEventRepository.createdEvents.front().eventType == "ObjectDetection");
    assert(metadataEventRepository.createdEvents.back().eventType == "DetectionUpdate");
}

}

int main() {
    processesRawMetadataThroughIngestionAndPersistence();
}
