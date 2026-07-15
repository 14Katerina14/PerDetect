using System;
using SecureZone.XProtectPlugin.Background;
using VideoOS.Platform;
using VideoOS.Platform.Data;
using VideoOS.Platform.Messaging;

namespace SecureZone.XProtectPlugin
{
    internal sealed class SecureZoneViolationEventPublisher
    {
        public void Publish(LineCrossingEventSnapshot sourceEvent, SecureZoneDecisionResponse decision)
        {
            var secureZoneEvent = new AnalyticsEvent
            {
                EventHeader = new EventHeader
                {
                    ID = Guid.NewGuid(),
                    Class = "SecureZone",
                    Type = "SecureZoneViolationConfirmed",
                    Timestamp = DateTime.Now,
                    Name = "SecureZone violation confirmed",
                    Message = BuildMessage(sourceEvent, decision),
                    Source = sourceEvent.Source
                }
            };

            EnvironmentManager.Instance.SendMessage(
                new Message(MessageId.Server.NewEventCommand) { Data = secureZoneEvent }
            );

            EnvironmentManager.Instance.Log(
                false,
                "SecureZone.DecisionBridge",
                "Raised SecureZoneViolationConfirmed for event '" + sourceEvent.EventId +
                "', zone '" + (decision.zoneId ?? string.Empty) + "'."
            );
        }

        private static string BuildMessage(
            LineCrossingEventSnapshot sourceEvent,
            SecureZoneDecisionResponse decision
        )
        {
            return "SecureZone violation confirmed for zone '" +
                   (decision.zoneId ?? "unknown") +
                   "' from source '" + sourceEvent.SourceName +
                   "'. Backend reason: " + (decision.message ?? "No reason supplied.") +
                   " Correlation event: " + sourceEvent.EventId + ".";
        }
    }
}
