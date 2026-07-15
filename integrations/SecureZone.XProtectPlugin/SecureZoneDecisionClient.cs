using System;
using System.Net.Http;
using System.Text;
using System.Threading.Tasks;
using System.Web.Script.Serialization;
using SecureZone.XProtectPlugin.Background;
using SecureZone.XProtectPlugin.Metadata;

namespace SecureZone.XProtectPlugin
{
    internal sealed class SecureZoneDecisionResponse
    {
        public bool accepted { get; set; }
        public string status { get; set; }
        public string decision { get; set; }
        public string zoneId { get; set; }
        public string sessionId { get; set; }
        public string employeeId { get; set; }
        public string message { get; set; }
        public string eventId { get; set; }
        public bool duplicate { get; set; }
    }

    internal sealed class SecureZoneDecisionRequest
    {
        public string eventId { get; set; }
        public string eventName { get; set; }
        public string sourceName { get; set; }
        public string receivedAt { get; set; }
        public string cameraId { get; set; }
        public string objectId { get; set; }
        public string action { get; set; }
    }

    internal sealed class SecureZoneDecisionClient : IDisposable
    {
        private readonly SecureZonePluginSettings settings;
        private readonly HttpClient httpClient;
        private readonly JavaScriptSerializer serializer = new JavaScriptSerializer();

        public SecureZoneDecisionClient(SecureZonePluginSettings settings)
        {
            this.settings = settings ?? throw new ArgumentNullException("settings");
            httpClient = new HttpClient { Timeout = settings.RequestTimeout };
        }

        public async Task<SecureZoneDecisionResponse> EvaluateAsync(LineCrossingEventSnapshot lineCrossingEvent)
        {
            var payload = new SecureZoneDecisionRequest
            {
                eventId = lineCrossingEvent.EventId,
                eventName = lineCrossingEvent.EventName,
                sourceName = lineCrossingEvent.SourceName,
                receivedAt = lineCrossingEvent.ReceivedAtUtc.ToString("O"),
                cameraId = lineCrossingEvent.CameraId,
                objectId = lineCrossingEvent.ObjectId,
                action = lineCrossingEvent.Action
            };

            using (var request = new HttpRequestMessage(HttpMethod.Post, settings.ApiEndpoint))
            {
                request.Content = new StringContent(serializer.Serialize(payload), Encoding.UTF8, "application/json");
                if (!string.IsNullOrEmpty(settings.ApiKey))
                {
                    request.Headers.Add("X-SecureZone-Api-Key", settings.ApiKey);
                }

                using (HttpResponseMessage response = await httpClient.SendAsync(request).ConfigureAwait(false))
                {
                    string body = await response.Content.ReadAsStringAsync().ConfigureAwait(false);
                    SecureZoneDecisionResponse result = serializer.Deserialize<SecureZoneDecisionResponse>(body);
                    if (result == null)
                    {
                        throw new InvalidOperationException("SecureZone API returned an empty decision response.");
                    }

                    if (!response.IsSuccessStatusCode)
                    {
                        throw new HttpRequestException(
                            "SecureZone API returned HTTP " + (int)response.StatusCode +
                            " with status '" + (result.status ?? "unknown") + "'."
                        );
                    }

                    return result;
                }
            }
        }

        public async Task ObserveAsync(HumanObjectObservation observation)
        {
            var payload = new
            {
                cameraId = observation.CameraId,
                objectId = observation.ObjectId,
                objectType = observation.ObjectType,
                observedAt = observation.ObservedAtUtc.ToString("O")
            };
            using (var request = new HttpRequestMessage(HttpMethod.Post, settings.ObjectObservationEndpoint))
            {
                request.Content = new StringContent(serializer.Serialize(payload), Encoding.UTF8, "application/json");
                if (!string.IsNullOrEmpty(settings.ApiKey))
                    request.Headers.Add("X-SecureZone-Api-Key", settings.ApiKey);
                using (HttpResponseMessage response = await httpClient.SendAsync(request).ConfigureAwait(false))
                {
                    if (!response.IsSuccessStatusCode)
                        throw new HttpRequestException("Object observation returned HTTP " + (int)response.StatusCode + ".");
                }
            }
        }

        public void Dispose()
        {
            httpClient.Dispose();
        }
    }
}
