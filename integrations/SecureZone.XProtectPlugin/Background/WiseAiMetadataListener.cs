using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;
using SecureZone.XProtectPlugin.Metadata;
using VideoOS.Platform;
using VideoOS.Platform.Data;
using VideoOS.Platform.Live;

namespace SecureZone.XProtectPlugin.Background
{
    internal sealed class WiseAiMetadataListener : IDisposable
    {
        private sealed class TrackedHuman
        {
            public HumanObjectObservation Observation { get; set; }
            public Item CameraSource { get; set; }
            public DateTime LastReceivedUtc { get; set; }
            public DateTime LastPublishedUtc { get; set; }
        }

        private readonly SecureZoneDecisionClient client;
        private readonly RecentLineCrossingCache lineCrossings;
        private readonly SecureZonePluginSettings settings;
        private readonly Action<LineCrossingEventSnapshot, SecureZoneDecisionResponse> decisionHandler;
        private readonly List<MetadataLiveSource> sources = new List<MetadataLiveSource>();
        private readonly object trackedHumansSync = new object();
        private readonly Dictionary<string, TrackedHuman> trackedHumans =
            new Dictionary<string, TrackedHuman>();
        private Timer lostObjectTimer;

        public WiseAiMetadataListener(
            SecureZoneDecisionClient client,
            RecentLineCrossingCache lineCrossings,
            SecureZonePluginSettings settings,
            Action<LineCrossingEventSnapshot, SecureZoneDecisionResponse> decisionHandler)
        {
            this.client = client;
            this.lineCrossings = lineCrossings;
            this.settings = settings;
            this.decisionHandler = decisionHandler;
        }

        public void Start()
        {
            foreach (Item item in Configuration.Instance.GetItemsByKind(Kind.Metadata))
            {
                Item cameraSource = ResolveCamera(item);
                string cameraId = cameraSource == null
                    ? item.FQID.ObjectId.ToString("D")
                    : cameraSource.FQID.ObjectId.ToString("D");
                var source = new MetadataLiveSource(item);
                source.LiveModeStart = true;
                source.Init();
                source.LiveContentEvent += (sender, content) => OnMetadata(cameraId, cameraSource, content);
                source.ErrorEvent += (sender, exception) => EnvironmentManager.Instance.Log(
                    true, "SecureZone.Metadata", "Metadata source error for '" + item.Name + "': " + exception.Message);
                sources.Add(source);
            }

            lostObjectTimer = new Timer(SweepLostObjects, null, TimeSpan.FromSeconds(1), TimeSpan.FromSeconds(1));
            EnvironmentManager.Instance.Log(false, "SecureZone.Metadata",
                "Listening to " + sources.Count + " XProtect metadata device(s). Human heartbeat=" +
                settings.MetadataHeartbeat.TotalSeconds + "s, lost-after=" +
                settings.MetadataLostAfter.TotalSeconds + "s.");
        }

        private static Item ResolveCamera(Item metadataItem)
        {
            try
            {
                foreach (Item related in metadataItem.GetRelated())
                {
                    if (related.FQID != null && related.FQID.Kind == Kind.Camera) return related;
                }
            }
            catch (Exception exception)
            {
                EnvironmentManager.Instance.Log(true, "SecureZone.Metadata",
                    "Could not resolve related camera for metadata device '" +
                    metadataItem.Name + "': " + exception.Message);
            }
            return null;
        }

        private void OnMetadata(string cameraId, Item cameraSource, MetadataLiveContent content)
        {
            if (content == null || content.Content == null) return;
            try
            {
                ParsedWiseAiMetadata parsed = WiseAiMetadataParser.Parse(
                    content.Content.GetMetadataString(), cameraId);
                foreach (RawLineCrossing crossing in parsed.LineCrossings) lineCrossings.Add(crossing);
                if (!parsed.HasVideoAnalyticsFrame) return;
                foreach (HumanObjectObservation human in parsed.Humans) TrackHuman(human, cameraSource);
            }
            catch (Exception exception)
            {
                EnvironmentManager.Instance.Log(true, "SecureZone.Metadata",
                    "Failed to parse WiseAI metadata: " + exception.Message);
            }
        }

        private void TrackHuman(HumanObjectObservation observation, Item cameraSource)
        {
            string key = observation.CameraId + "|" + observation.ObjectId;
            bool publish = false;
            DateTime now = DateTime.UtcNow;
            lock (trackedHumansSync)
            {
                TrackedHuman tracked;
                if (!trackedHumans.TryGetValue(key, out tracked))
                {
                    tracked = new TrackedHuman();
                    trackedHumans[key] = tracked;
                    publish = true;
                }
                else if (now - tracked.LastPublishedUtc >= settings.MetadataHeartbeat)
                {
                    publish = true;
                }

                tracked.Observation = observation;
                tracked.CameraSource = cameraSource;
                tracked.LastReceivedUtc = now;
                if (publish) tracked.LastPublishedUtc = now;
            }

            if (publish) Task.Run(() => PublishObservationAsync(observation, cameraSource));
        }

        private void SweepLostObjects(object state)
        {
            List<TrackedHuman> lost;
            DateTime now = DateTime.UtcNow;
            lock (trackedHumansSync)
            {
                lost = trackedHumans.Values
                    .Where(x => now - x.LastReceivedUtc >= settings.MetadataLostAfter)
                    .ToList();
                foreach (TrackedHuman tracked in lost)
                {
                    trackedHumans.Remove(tracked.Observation.CameraId + "|" + tracked.Observation.ObjectId);
                }
            }

            foreach (TrackedHuman tracked in lost)
            {
                var observation = new HumanObjectObservation
                {
                    CameraId = tracked.Observation.CameraId,
                    ObjectId = tracked.Observation.ObjectId,
                    ObjectType = tracked.Observation.ObjectType,
                    ObservedAtUtc = now,
                    Status = "lost"
                };
                Task.Run(() => PublishObservationAsync(observation, tracked.CameraSource));
            }
        }

        private async Task PublishObservationAsync(HumanObjectObservation observation, Item cameraSource)
        {
            try
            {
                SecureZoneDecisionResponse decision = await client.ObserveAsync(observation).ConfigureAwait(false);
                var snapshot = new LineCrossingEventSnapshot
                {
                    EventId = string.IsNullOrEmpty(decision.eventId)
                        ? "IDENTITY-" + observation.CameraId + "-" + observation.ObjectId
                        : decision.eventId,
                    EventName = "SecureZone.UnidentifiedPresence",
                    SourceName = cameraSource == null ? observation.CameraId : cameraSource.Name,
                    ReceivedAtUtc = observation.ObservedAtUtc,
                    CameraId = observation.CameraId,
                    ObjectId = observation.ObjectId,
                    Action = observation.Status,
                    Source = cameraSource == null
                        ? null
                        : new EventSource { FQID = cameraSource.FQID, Name = cameraSource.Name }
                };
                decisionHandler(snapshot, decision);
            }
            catch (Exception exception)
            {
                EnvironmentManager.Instance.Log(true, "SecureZone.Metadata",
                    "Failed to process Human object '" + observation.ObjectId +
                    "' with status '" + observation.Status + "': " + exception.Message);
            }
        }

        public void Dispose()
        {
            if (lostObjectTimer != null)
            {
                lostObjectTimer.Dispose();
                lostObjectTimer = null;
            }
            foreach (MetadataLiveSource source in sources) source.Close();
            sources.Clear();
            lock (trackedHumansSync) trackedHumans.Clear();
        }
    }
}
