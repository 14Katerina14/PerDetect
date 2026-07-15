using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using SecureZone.XProtectPlugin.Metadata;
using VideoOS.Platform;
using VideoOS.Platform.Live;

namespace SecureZone.XProtectPlugin.Background
{
    internal sealed class WiseAiMetadataListener : IDisposable
    {
        private readonly SecureZoneDecisionClient client;
        private readonly RecentLineCrossingCache lineCrossings;
        private readonly List<MetadataLiveSource> sources = new List<MetadataLiveSource>();
        private readonly object observationsSync = new object();
        private readonly Dictionary<string, DateTime> recentObservations = new Dictionary<string, DateTime>();

        public WiseAiMetadataListener(SecureZoneDecisionClient client, RecentLineCrossingCache lineCrossings)
        {
            this.client = client;
            this.lineCrossings = lineCrossings;
        }

        public void Start()
        {
            foreach (Item item in Configuration.Instance.GetItemsByKind(Kind.Metadata))
            {
                var source = new MetadataLiveSource(item);
                source.LiveModeStart = true;
                source.Init();
                string cameraId = ResolveCameraId(item);
                source.LiveContentEvent += (sender, content) => OnMetadata(cameraId, content);
                source.ErrorEvent += (sender, exception) => EnvironmentManager.Instance.Log(
                    true, "SecureZone.Metadata", "Metadata source error for '" + item.Name + "': " + exception.Message);
                sources.Add(source);
            }
            EnvironmentManager.Instance.Log(false, "SecureZone.Metadata",
                "Listening to " + sources.Count + " XProtect metadata device(s).");
        }

        private static string ResolveCameraId(Item metadataItem)
        {
            try
            {
                foreach (Item related in metadataItem.GetRelated())
                {
                    if (related.FQID != null && related.FQID.Kind == Kind.Camera)
                    {
                        return related.FQID.ObjectId.ToString("D");
                    }
                }
            }
            catch (Exception exception)
            {
                EnvironmentManager.Instance.Log(true, "SecureZone.Metadata",
                    "Could not resolve related camera for metadata device '" +
                    metadataItem.Name + "': " + exception.Message);
            }

            return metadataItem.FQID.ObjectId.ToString("D");
        }

        private void OnMetadata(string cameraId, MetadataLiveContent content)
        {
            if (content == null || content.Content == null) return;
            try
            {
                ParsedWiseAiMetadata parsed = WiseAiMetadataParser.Parse(
                    content.Content.GetMetadataString(), cameraId);
                foreach (RawLineCrossing crossing in parsed.LineCrossings) lineCrossings.Add(crossing);
                foreach (HumanObjectObservation human in parsed.Humans)
                {
                    if (ShouldPublish(human)) Task.Run(() => PublishObservationAsync(human));
                }
            }
            catch (Exception exception)
            {
                EnvironmentManager.Instance.Log(true, "SecureZone.Metadata",
                    "Failed to parse WiseAI metadata: " + exception.Message);
            }
        }

        private bool ShouldPublish(HumanObjectObservation observation)
        {
            string key = observation.CameraId + "|" + observation.ObjectId;
            lock (observationsSync)
            {
                DateTime lastPublished;
                if (recentObservations.TryGetValue(key, out lastPublished) &&
                    observation.ObservedAtUtc - lastPublished < TimeSpan.FromMinutes(2))
                {
                    return false;
                }

                recentObservations[key] = observation.ObservedAtUtc;
                return true;
            }
        }

        private async Task PublishObservationAsync(HumanObjectObservation observation)
        {
            try
            {
                await client.ObserveAsync(observation).ConfigureAwait(false);
            }
            catch (Exception exception)
            {
                EnvironmentManager.Instance.Log(true, "SecureZone.Metadata",
                    "Failed to publish Human object '" + observation.ObjectId +
                    "': " + exception.Message);
            }
        }

        public void Dispose()
        {
            foreach (MetadataLiveSource source in sources) source.Close();
            sources.Clear();
        }
    }
}
