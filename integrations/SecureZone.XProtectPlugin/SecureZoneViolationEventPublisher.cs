using System;
using SecureZone.XProtectPlugin.Background;
using VideoOS.Platform;
using VideoOS.Platform.Data;
using VideoOS.Platform.Messaging;

namespace SecureZone.XProtectPlugin
{
    internal sealed class SecureZoneViolationEventPublisher : IDecisionEventPublisher
    {
        public void Publish(LineCrossingEventSnapshot sourceEvent, SecureZoneDecisionResponse decision)
        {
            PublishEvent(
                sourceEvent,
                decision,
                "SecureZoneViolationConfirmed",
                "SecureZone violation confirmed",
                "Raised SecureZoneViolationConfirmed"
            );
        }

        public void PublishCleared(LineCrossingEventSnapshot sourceEvent, SecureZoneDecisionResponse decision)
        {
            PublishEvent(
                sourceEvent,
                decision,
                "SecureZoneViolationCleared",
                "SecureZone violation cleared",
                "Raised SecureZoneViolationCleared"
            );
        }

        private static void PublishEvent(
            LineCrossingEventSnapshot sourceEvent,
            SecureZoneDecisionResponse decision,
            string eventType,
            string eventName,
            string logMessage)
        {
            var secureZoneEvent = new AnalyticsEvent
            {
                EventHeader = new EventHeader
                {
                    ID = Guid.NewGuid(),
                    Class = "SecureZone",
                    Type = eventType,
                    Timestamp = DateTime.Now,
                    Name = eventName,
                    Message = BuildMessage(eventName, sourceEvent, decision),
                    Source = sourceEvent.Source
                }
            };

            EnvironmentManager.Instance.SendMessage(
                new Message(MessageId.Server.NewEventCommand) { Data = secureZoneEvent }
            );

            EnvironmentManager.Instance.Log(
                false,
                "SecureZone.DecisionBridge",
                logMessage + " for event '" + sourceEvent.EventId +
                "', zone '" + (decision.zoneId ?? string.Empty) + "'."
            );
        }

        private static string BuildMessage(
            string eventName,
            LineCrossingEventSnapshot sourceEvent,
            SecureZoneDecisionResponse decision
        )
        {
            return eventName + " for zone '" +
                   (decision.zoneId ?? "unknown") +
                   "' from source '" + sourceEvent.SourceName +
                   "'. Backend reason: " + (decision.message ?? "No reason supplied.") +
                   " Correlation event: " + sourceEvent.EventId + ".";
        }
    }
}
