using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Xml.Linq;

namespace SecureZone.XProtectPlugin.Metadata
{
    internal sealed class HumanObjectObservation
    {
        public string CameraId { get; set; }
        public string ObjectId { get; set; }
        public string ObjectType { get; set; }
        public DateTime ObservedAtUtc { get; set; }
    }

    internal sealed class RawLineCrossing
    {
        public string CameraId { get; set; }
        public string ObjectId { get; set; }
        public string Action { get; set; }
        public string RuleName { get; set; }
        public DateTime OccurredAtUtc { get; set; }
    }

    internal sealed class ParsedWiseAiMetadata
    {
        public List<HumanObjectObservation> Humans { get; } = new List<HumanObjectObservation>();
        public List<RawLineCrossing> LineCrossings { get; } = new List<RawLineCrossing>();
    }

    internal static class WiseAiMetadataParser
    {
        public static ParsedWiseAiMetadata Parse(string xml, string cameraId)
        {
            var result = new ParsedWiseAiMetadata();
            if (string.IsNullOrWhiteSpace(xml) || string.IsNullOrWhiteSpace(cameraId)) return result;

            XDocument document = XDocument.Parse(xml, LoadOptions.None);
            foreach (XElement frame in document.Descendants().Where(x => x.Name.LocalName == "Frame"))
            {
                DateTime observedAt = ReadUtc(frame.Attribute("UtcTime") == null ? null : frame.Attribute("UtcTime").Value);
                foreach (XElement cameraObject in frame.Elements().Where(x => x.Name.LocalName == "Object"))
                {
                    string objectId = Attribute(cameraObject, "ObjectId");
                    string objectType = ReadObjectType(cameraObject);
                    if ((string.Equals(objectType, "Human", StringComparison.OrdinalIgnoreCase) ||
                         string.Equals(objectType, "Person", StringComparison.OrdinalIgnoreCase)) &&
                        objectId.Length > 0)
                    {
                        result.Humans.Add(new HumanObjectObservation
                        {
                            CameraId = cameraId,
                            ObjectId = objectId,
                            ObjectType = objectType,
                            ObservedAtUtc = observedAt
                        });
                    }
                }
            }

            foreach (XElement notification in document.Descendants()
                .Where(x => x.Name.LocalName == "NotificationMessage"))
            {
                string topic = notification.Descendants()
                    .Where(x => x.Name.LocalName == "Topic")
                    .Select(x => x.Value)
                    .FirstOrDefault() ?? string.Empty;
                if (topic.IndexOf("LineCrossing", StringComparison.OrdinalIgnoreCase) < 0) continue;

                XElement message = notification.Descendants().FirstOrDefault(x => x.Name.LocalName == "Message");
                if (message == null) continue;
                string objectId = Item(message, "ObjectId");
                if (objectId.Length == 0) continue;
                result.LineCrossings.Add(new RawLineCrossing
                {
                    CameraId = cameraId,
                    ObjectId = objectId,
                    Action = Item(message, "Action"),
                    RuleName = Item(message, "RuleName"),
                    OccurredAtUtc = ReadUtc(Attribute(message, "UtcTime"))
                });
            }
            return result;
        }

        private static string Item(XElement parent, string name)
        {
            XElement item = parent.Descendants().FirstOrDefault(x =>
                x.Name.LocalName == "SimpleItem" && Attribute(x, "Name") == name);
            return item == null ? string.Empty : Attribute(item, "Value");
        }

        private static string ReadObjectType(XElement cameraObject)
        {
            foreach (XElement element in cameraObject.Descendants())
            {
                if (element.Name.LocalName == "Type")
                {
                    string value = element.Value.Trim();
                    if (value.Length > 0) return value;
                    value = Attribute(element, "Type");
                    if (value.Length > 0) return value;
                }

                if (element.Name.LocalName == "Class")
                {
                    string value = Attribute(element, "Type");
                    if (value.Length > 0) return value;
                    value = element.Value.Trim();
                    if (value.Length > 0) return value;
                }

                if (element.Name.LocalName == "SimpleItem" &&
                    (Attribute(element, "Name") == "ObjectClass" ||
                     Attribute(element, "Name") == "ObjectType"))
                {
                    return Attribute(element, "Value");
                }
            }

            return string.Empty;
        }

        private static string Attribute(XElement element, string name)
        {
            XAttribute attribute = element.Attribute(name);
            return attribute == null ? string.Empty : attribute.Value;
        }

        private static DateTime ReadUtc(string value)
        {
            DateTime parsed;
            return DateTime.TryParse(value, CultureInfo.InvariantCulture,
                DateTimeStyles.AssumeUniversal | DateTimeStyles.AdjustToUniversal, out parsed)
                ? parsed : DateTime.UtcNow;
        }
    }
}
