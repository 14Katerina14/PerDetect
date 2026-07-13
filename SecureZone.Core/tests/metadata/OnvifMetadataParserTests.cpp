#include "securezone/metadata/OnvifMetadataParser.h"

#include <cassert>
#include <chrono>
#include <string>

namespace {

constexpr const char* AnalyticsMetadata =
    R"(<?xml version="1.0" encoding="UTF-8"?><tt:MetadataStream xmlns:tt="http://www.onvif.org/ver10/schema" xmlns:ttr="https://www.onvif.org/ver20/analytics/radiometry" xmlns:wsnt="http://docs.oasis-open.org/wsn/b-2" xmlns:tns1="http://www.onvif.org/ver10/topics" xmlns:tnssamsung="http://www.samsungcctv.com/2011/event/topics" xmlns:fc="http://www.onvif.org/ver20/analytics/humanface" xmlns:bd="http://www.onvif.org/ver20/analytics/humanbody"><tt:VideoAnalytics><tt:Frame UtcTime="2026-07-13T13:24:44.592Z" Source="WiseAI"><tt:Transformation><tt:Translate x="-1.0" y="1.0"/><tt:Scale x="0.001563" y="-0.002083"/></tt:Transformation><tt:Object ObjectId="15"><tt:Appearance><tt:Shape><tt:BoundingBox left="672.0" top="169.0" right="1260.0" bottom="944.0"/><tt:CenterOfGravity x="966.0" y="556.5"/></tt:Shape><tt:Class><tt:ClassCandidate><tt:Type>Human</tt:Type><tt:Likelihood>0.54</tt:Likelihood></tt:ClassCandidate><tt:Type Likelihood="0.54">Human</tt:Type></tt:Class><tt:ProximateObjects><tt:ProximateObject Id="0" Distance="0.000000"/></tt:ProximateObjects></tt:Appearance></tt:Object></tt:Frame></tt:VideoAnalytics></tt:MetadataStream>)";

constexpr const char* DetectionEventMetadata =
    R"(<?xml version="1.0" encoding="UTF-8"?><tt:MetadataStream xmlns:tt="http://www.onvif.org/ver10/schema" xmlns:ttr="https://www.onvif.org/ver20/analytics/radiometry" xmlns:wsnt="http://docs.oasis-open.org/wsn/b-2" xmlns:tns1="http://www.onvif.org/ver10/topics" xmlns:tnssamsung="http://www.samsungcctv.com/2011/event/topics" xmlns:fc="http://www.onvif.org/ver20/analytics/humanface" xmlns:bd="http://www.onvif.org/ver20/analytics/humanbody"><tt:Event><wsnt:NotificationMessage><wsnt:Topic Dialect="http://www.onvif.org/ver10/tev/topicExpression/ConcreteSet">tns1:OpenApp/WiseAI/ObjectDetection</wsnt:Topic><wsnt:Message><tt:Message UtcTime="2026-07-13T13:24:45.593Z" PropertyOperation="Changed"><tt:Source><tt:SimpleItem Name="VideoSourceToken" Value="vs-0"/><tt:SimpleItem Name="RuleName" Value="ObjectDetectionRule-1"/></tt:Source><tt:Key></tt:Key><tt:Data><tt:SimpleItem Name="State" Value="true"/><tt:SimpleItem Name="ClassTypes" Value="Person"/></tt:Data></tt:Message></wsnt:Message></wsnt:NotificationMessage></tt:Event></tt:MetadataStream>)";

void parsesHumanDetectionFromWiseAiFrame() {
    const auto frame = securezone::metadata::OnvifMetadataParser{}.parse("CAM-001", AnalyticsMetadata);

    assert(frame.cameraId == "CAM-001");
    assert(frame.source == "WiseAI");
    assert(frame.detections.size() == 1);

    const auto& detection = frame.detections.front();
    assert(detection.trackId == "15");
    assert(detection.cameraId == "CAM-001");
    assert(detection.objectClass == securezone::domain::ObjectClass::Person);
    assert(detection.confidence == 0.54);
    assert(detection.bbox.x == 672.0);
    assert(detection.bbox.y == 169.0);
    assert(detection.bbox.width == 588.0);
    assert(detection.bbox.height == 775.0);
    assert(detection.timestamp != std::chrono::system_clock::time_point{});
}

void parsesObjectDetectionEvent() {
    const auto frame = securezone::metadata::OnvifMetadataParser{}.parse("CAM-001", DetectionEventMetadata);

    assert(frame.cameraId == "CAM-001");
    assert(frame.detections.empty());
    assert(frame.events.size() == 1);

    const auto& event = frame.events.front();
    assert(event.cameraId == "CAM-001");
    assert(event.objectClass == "Person");
    assert(event.eventType == "ObjectDetection");
    assert(event.timestamp != std::chrono::system_clock::time_point{});
}

}

int main() {
    parsesHumanDetectionFromWiseAiFrame();
    parsesObjectDetectionEvent();
}
