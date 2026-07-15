using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using VideoOS.Platform;
using VideoOS.Platform.Background;
using VideoOS.Platform.Data;
using VideoOS.Platform.Messaging;
using SecureZone.XProtectPlugin.Metadata;

namespace SecureZone.XProtectPlugin.Background
{
    public class SecureZoneEventServerBackgroundPlugin : BackgroundPlugin
    {
        private object eventReceiver;
        private SecureZoneDecisionClient decisionClient;
        private SecureZoneViolationEventPublisher violationPublisher;
        private RecentLineCrossingCache lineCrossingCache;
        private WiseAiMetadataListener metadataListener;

        public override Guid Id
        {
            get { return SecureZonePluginDefinition.BackgroundPluginId; }
        }

        public override string Name
        {
            get { return "SecureZone Decision Bridge"; }
        }

        public override void Init()
        {
            SecureZonePluginSettings settings = SecureZonePluginSettings.Load();
            decisionClient = new SecureZoneDecisionClient(settings);
            violationPublisher = new SecureZoneViolationEventPublisher();
            lineCrossingCache = new RecentLineCrossingCache();
            metadataListener = new WiseAiMetadataListener(decisionClient, lineCrossingCache);
            metadataListener.Start();

            eventReceiver = EnvironmentManager.Instance.RegisterReceiver(
                NewEventHandler,
                new MessageIdFilter(MessageId.Server.NewEventIndication)
            );

            EnvironmentManager.Instance.Log(
                false,
                "SecureZone.DecisionBridge",
                "Listening for WiseAI LineCrossing events. Backend endpoint: " + settings.ApiEndpoint + "."
            );
        }

        public override void Close()
        {
            if (eventReceiver != null)
            {
                EnvironmentManager.Instance.UnRegisterReceiver(eventReceiver);
                eventReceiver = null;
            }

            if (metadataListener != null)
            {
                metadataListener.Dispose();
                metadataListener = null;
            }
            if (decisionClient != null)
            {
                decisionClient.Dispose();
                decisionClient = null;
            }
        }

        public override List<EnvironmentType> TargetEnvironments
        {
            get { return new List<EnvironmentType> { EnvironmentType.Service }; }
        }

        private object NewEventHandler(Message message, FQID destination, FQID source)
        {
            BaseEvent baseEvent = message.Data as BaseEvent;
            if (baseEvent == null || baseEvent.EventHeader == null)
            {
                return null;
            }

            EventHeader header = baseEvent.EventHeader;
            string eventName = header.Name ?? string.Empty;
            string eventType = header.Type ?? string.Empty;
            string eventMessage = header.Message ?? string.Empty;

            if (!IsWiseAiLineCrossing(eventName, eventType, eventMessage))
            {
                return null;
            }

            var snapshot = new LineCrossingEventSnapshot
            {
                EventId = header.ID == Guid.Empty ? Guid.NewGuid().ToString("D") : header.ID.ToString("D"),
                EventName = eventName,
                SourceName = header.Source == null ? string.Empty : header.Source.Name ?? string.Empty,
                ReceivedAtUtc = header.Timestamp == DateTime.MinValue
                    ? DateTime.UtcNow
                    : header.Timestamp.ToUniversalTime(),
                CameraId = header.Source == null || header.Source.FQID == null
                    ? string.Empty
                    : header.Source.FQID.ObjectId.ToString("D"),
                Source = header.Source
            };
            RawLineCrossing raw = lineCrossingCache.TakeClosest(snapshot.ReceivedAtUtc, snapshot.CameraId);
            if (raw != null)
            {
                snapshot.CameraId = raw.CameraId;
                snapshot.ObjectId = raw.ObjectId;
                snapshot.Action = raw.Action;
            }

            Task.Run(() => ProcessLineCrossingAsync(snapshot));
            return null;
        }

        private async Task ProcessLineCrossingAsync(LineCrossingEventSnapshot snapshot)
        {
            try
            {
                SecureZoneDecisionResponse decision = await decisionClient.EvaluateAsync(snapshot).ConfigureAwait(false);
                if (!decision.duplicate &&
                    decision.accepted &&
                    string.Equals(decision.decision, "violation", StringComparison.OrdinalIgnoreCase))
                {
                    if (snapshot.Source == null)
                    {
                        EnvironmentManager.Instance.Log(
                            true,
                            "SecureZone.DecisionBridge",
                            "Backend confirmed violation, but the XProtect event has no source. Event '" +
                            snapshot.EventId + "' was not raised."
                        );
                        return;
                    }

                    violationPublisher.Publish(snapshot, decision);
                    return;
                }

                EnvironmentManager.Instance.Log(
                    false,
                    "SecureZone.DecisionBridge",
                    "No violation event raised for event '" + snapshot.EventId +
                    "'. Backend decision='" + (decision.decision ?? "none") +
                    "', status='" + (decision.status ?? "unknown") + "'."
                );
            }
            catch (Exception exception)
            {
                EnvironmentManager.Instance.Log(
                    true,
                    "SecureZone.DecisionBridge",
                    "SecureZone API decision failed for event '" + snapshot.EventId +
                    "'. No confirmed violation event was raised. " + exception.Message
                );
            }
        }

        private static bool IsWiseAiLineCrossing(string eventName, string eventType, string eventMessage)
        {
            return ContainsIgnoreCase(eventName, "WiseAI.LineCrossing") ||
                   ContainsIgnoreCase(eventType, "WiseAI.LineCrossing") ||
                   ContainsIgnoreCase(eventMessage, "WiseAI.LineCrossing") ||
                   ContainsIgnoreCase(eventName, "LineCrossing") ||
                   ContainsIgnoreCase(eventType, "LineCrossing");
        }

        private static bool ContainsIgnoreCase(string value, string token)
        {
            return value != null && value.IndexOf(token, StringComparison.OrdinalIgnoreCase) >= 0;
        }
    }
}
