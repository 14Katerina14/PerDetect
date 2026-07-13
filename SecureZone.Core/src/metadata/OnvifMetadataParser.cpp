#include "securezone/metadata/OnvifMetadataParser.h"

#include <algorithm>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>

namespace securezone::metadata {

namespace {

std::string firstMatch(const std::string& input, const std::regex& pattern) {
    std::smatch match;
    if (!std::regex_search(input, match, pattern)) {
        return {};
    }

    return match[1].str();
}

double parseDouble(const std::string& value) {
    if (value.empty()) {
        return 0.0;
    }

    return std::stod(value);
}

domain::ObjectClass parseObjectClass(const std::string& value) {
    if (value == "Human" || value == "Person") {
        return domain::ObjectClass::Person;
    }

    if (value == "Vehicle") {
        return domain::ObjectClass::Vehicle;
    }

    return domain::ObjectClass::Unknown;
}

std::chrono::system_clock::time_point parseUtcTimestamp(const std::string& value) {
    if (value.empty()) {
        return {};
    }

    std::tm tm{};
    std::istringstream stream{value.substr(0, 19)};
    stream >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");
    if (stream.fail()) {
        return {};
    }

#if defined(_WIN32)
    const auto seconds = _mkgmtime(&tm);
#else
    const auto seconds = timegm(&tm);
#endif
    if (seconds == -1) {
        return {};
    }

    return std::chrono::system_clock::from_time_t(seconds);
}

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

domain::BoundingBox parseBoundingBox(const std::string& objectXml) {
    static const std::regex bboxPattern{
        "<tt:BoundingBox\\s+left=\"([^\"]+)\"\\s+top=\"([^\"]+)\"\\s+right=\"([^\"]+)\"\\s+bottom=\"([^\"]+)\""
    };

    std::smatch match;
    if (!std::regex_search(objectXml, match, bboxPattern)) {
        return {};
    }

    const auto left = parseDouble(match[1].str());
    const auto top = parseDouble(match[2].str());
    const auto right = parseDouble(match[3].str());
    const auto bottom = parseDouble(match[4].str());

    return {left, top, right - left, bottom - top};
}

std::vector<domain::Detection> parseDetections(
    const std::string& cameraId,
    std::chrono::system_clock::time_point timestamp,
    const std::string& rawMetadata
) {
    static const std::regex objectPattern{"<tt:Object\\s+ObjectId=\"([^\"]+)\">(.*?)</tt:Object>"};
    static const std::regex typeElementPattern{"<tt:Type(?:\\s+Likelihood=\"([^\"]+)\")?>([^<]+)</tt:Type>"};
    static const std::regex candidateLikelihoodPattern{"<tt:Likelihood>([^<]+)</tt:Likelihood>"};

    std::vector<domain::Detection> detections;
    for (auto it = std::sregex_iterator(rawMetadata.begin(), rawMetadata.end(), objectPattern);
         it != std::sregex_iterator{};
         ++it) {
        const auto objectId = (*it)[1].str();
        const auto objectXml = (*it)[2].str();

        std::smatch typeMatch;
        std::string className;
        double confidence = 0.0;
        if (std::regex_search(objectXml, typeMatch, typeElementPattern)) {
            confidence = parseDouble(typeMatch[1].str());
            className = typeMatch[2].str();
        }

        if (confidence == 0.0) {
            confidence = parseDouble(firstMatch(objectXml, candidateLikelihoodPattern));
        }

        detections.push_back(domain::Detection{
            objectId,
            cameraId,
            parseObjectClass(className),
            parseBoundingBox(objectXml),
            confidence,
            timestamp
        });
    }

    return detections;
}

std::vector<domain::MetadataEvent> parseEvents(
    const std::string& cameraId,
    std::chrono::system_clock::time_point timestamp,
    const std::string& rawMetadata,
    const std::vector<domain::Detection>& detections
) {
    static const std::regex topicPattern{"<wsnt:Topic[^>]*>([^<]+)</wsnt:Topic>"};
    static const std::regex classTypesPattern{"Name=\"ClassTypes\"\\s+Value=\"([^\"]+)\""};

    const auto topic = firstMatch(rawMetadata, topicPattern);
    if (topic.empty()) {
        return {};
    }

    const auto classTypes = firstMatch(rawMetadata, classTypesPattern);
    const auto eventType = topic.find("ObjectDetection") != std::string::npos
        ? "ObjectDetection"
        : topic;

    std::vector<domain::MetadataEvent> events;
    if (detections.empty()) {
        events.push_back(domain::MetadataEvent{
            cameraId + ":" + eventType,
            cameraId,
            {},
            timestamp,
            classTypes,
            {},
            {},
            eventType
        });
        return events;
    }

    events.reserve(detections.size());
    for (const auto& detection : detections) {
        events.push_back(domain::MetadataEvent{
            cameraId + ":" + detection.trackId + ":" + eventType,
            cameraId,
            detection.trackId,
            timestamp,
            objectClassToString(detection.objectClass),
            detection.bbox,
            {},
            eventType
        });
    }

    return events;
}

}

ParsedMetadataFrame OnvifMetadataParser::parse(
    const std::string& cameraId,
    const std::string& rawMetadata
) const {
    static const std::regex framePattern{"<tt:Frame\\s+UtcTime=\"([^\"]+)\"\\s+Source=\"([^\"]+)\""};
    static const std::regex messagePattern{"<tt:Message\\s+UtcTime=\"([^\"]+)\""};

    const auto timestampText = firstMatch(rawMetadata, framePattern).empty()
        ? firstMatch(rawMetadata, messagePattern)
        : firstMatch(rawMetadata, framePattern);
    const auto source = firstMatch(rawMetadata, std::regex{"Source=\"([^\"]+)\""});
    const auto timestamp = parseUtcTimestamp(timestampText);
    const auto detections = parseDetections(cameraId, timestamp, rawMetadata);

    return ParsedMetadataFrame{
        cameraId,
        source,
        timestamp,
        detections,
        parseEvents(cameraId, timestamp, rawMetadata, detections)
    };
}

}
