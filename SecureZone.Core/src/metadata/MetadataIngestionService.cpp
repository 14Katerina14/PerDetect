#include "securezone/metadata/MetadataIngestionService.h"

#include <chrono>
#include <sstream>
#include <string>
#include <utility>

namespace securezone::metadata {

namespace {

std::string objectClassToString(domain::ObjectClass objectClass) {
    switch (objectClass) {
        case domain::ObjectClass::Person:
            return "Person";
        case domain::ObjectClass::Vehicle:
            return "Vehicle";
        case domain::ObjectClass::Unknown:
            return "Unknown";
    }

    return "Unknown";
}

std::string eventIdForDetection(
    const std::string& cameraId,
    const domain::Detection& detection
) {
    const auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(
        detection.timestamp.time_since_epoch()
    ).count();

    std::ostringstream eventId;
    eventId << cameraId << ':' << detection.trackId << ":DetectionUpdate:" << milliseconds;
    return eventId.str();
}

domain::CameraTrack cameraTrackFromDetection(const domain::Detection& detection) {
    return domain::CameraTrack{
        detection.trackId,
        detection.cameraId,
        detection.timestamp,
        detection.timestamp,
        {},
        objectClassToString(detection.objectClass),
        detection.bbox,
        "active"
    };
}

domain::MetadataEvent metadataEventFromDetection(const domain::Detection& detection) {
    return domain::MetadataEvent{
        eventIdForDetection(detection.cameraId, detection),
        detection.cameraId,
        detection.trackId,
        detection.timestamp,
        objectClassToString(detection.objectClass),
        detection.bbox,
        {},
        "DetectionUpdate"
    };
}

}

MetadataIngestionService::MetadataIngestionService(const IMetadataParser& parser)
    : parser_{parser} {
}

MetadataIngestionResult MetadataIngestionService::ingest(
    const std::string& cameraId,
    const std::string& rawMetadata
) const {
    const auto frame = parser_.parse(cameraId, rawMetadata);

    MetadataIngestionResult result{};
    result.detections = frame.detections;
    result.metadataEvents = frame.events;

    result.cameraTracks.reserve(frame.detections.size());
    result.metadataEvents.reserve(result.metadataEvents.size() + frame.detections.size());

    for (const auto& detection : frame.detections) {
        result.cameraTracks.push_back(cameraTrackFromDetection(detection));
        result.metadataEvents.push_back(metadataEventFromDetection(detection));
    }

    return result;
}

}
