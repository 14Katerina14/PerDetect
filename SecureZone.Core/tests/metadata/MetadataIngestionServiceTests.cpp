#include "securezone/metadata/MetadataIngestionService.h"

#include <cassert>
#include <chrono>
#include <string>

namespace {

class FakeMetadataParser final : public securezone::metadata::IMetadataParser {
public:
    securezone::metadata::ParsedMetadataFrame parse(
        const std::string& cameraId,
        const std::string& rawMetadata
    ) const override {
        assert(cameraId == "CAM-001");
        assert(rawMetadata == "<metadata/>");

        const auto timestamp = std::chrono::system_clock::time_point{
            std::chrono::milliseconds{1783960000000}
        };

        securezone::domain::Detection detection{};
        detection.trackId = "15";
        detection.cameraId = cameraId;
        detection.objectClass = securezone::domain::ObjectClass::Person;
        detection.bbox = {672.0, 169.0, 588.0, 775.0};
        detection.confidence = 0.54;
        detection.timestamp = timestamp;

        securezone::domain::MetadataEvent metadataEvent{};
        metadataEvent.eventId = "CAM-001:ObjectDetection";
        metadataEvent.cameraId = cameraId;
        metadataEvent.timestamp = timestamp;
        metadataEvent.objectClass = "Person";
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

void convertsParsedDetectionsToTracksAndEvents() {
    const FakeMetadataParser parser{};
    const securezone::metadata::MetadataIngestionService service{parser};

    const auto result = service.ingest("CAM-001", "<metadata/>");

    assert(result.detections.size() == 1);
    assert(result.cameraTracks.size() == 1);
    assert(result.metadataEvents.size() == 2);

    const auto& track = result.cameraTracks.front();
    assert(track.trackId == "15");
    assert(track.cameraId == "CAM-001");
    assert(track.objectClass == "Person");
    assert(track.status == "active");
    assert(track.bbox.x == 672.0);
    assert(track.bbox.y == 169.0);
    assert(track.bbox.width == 588.0);
    assert(track.bbox.height == 775.0);

    const auto& detectionEvent = result.metadataEvents.back();
    assert(detectionEvent.trackId == "15");
    assert(detectionEvent.cameraId == "CAM-001");
    assert(detectionEvent.objectClass == "Person");
    assert(detectionEvent.eventType == "DetectionUpdate");
}

}

int main() {
    convertsParsedDetectionsToTracksAndEvents();
}
