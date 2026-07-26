using System;

namespace SecureZone.XProtectPlugin
{
    internal sealed class SecureZonePluginSettings
    {
        private const string DefaultApiUrl = "http://127.0.0.1:8080/api/xprotect/line-crossing";
        private const int DefaultTimeoutSeconds = 5;

        public Uri ApiEndpoint { get; private set; }
        public Uri ObjectObservationEndpoint { get; private set; }
        public string ApiKey { get; private set; }
        public TimeSpan RequestTimeout { get; private set; }
        public TimeSpan MetadataHeartbeat { get; private set; }
        public TimeSpan MetadataLostAfter { get; private set; }

        public static SecureZonePluginSettings Load()
        {
            string apiUrl = ReadEnvironmentVariable("SECUREZONE_API_URL", DefaultApiUrl);
            Uri endpoint;
            if (!Uri.TryCreate(apiUrl, UriKind.Absolute, out endpoint) ||
                (endpoint.Scheme != Uri.UriSchemeHttp && endpoint.Scheme != Uri.UriSchemeHttps))
            {
                throw new InvalidOperationException("SECUREZONE_API_URL must be an absolute HTTP or HTTPS URL.");
            }

            int heartbeatSeconds = ReadPositiveInt("SECUREZONE_METADATA_HEARTBEAT_SECONDS", 5);
            int lostAfterSeconds = ReadPositiveInt("SECUREZONE_METADATA_LOST_AFTER_SECONDS", 8);
            if (lostAfterSeconds <= heartbeatSeconds) lostAfterSeconds = heartbeatSeconds + 3;

            return new SecureZonePluginSettings
            {
                ApiEndpoint = endpoint,
                ObjectObservationEndpoint = new Uri(endpoint, "/api/xprotect/object-observations"),
                ApiKey = ReadEnvironmentVariable("SECUREZONE_XPROTECT_API_KEY", string.Empty),
                RequestTimeout = TimeSpan.FromSeconds(ReadPositiveInt("SECUREZONE_API_TIMEOUT_SECONDS", DefaultTimeoutSeconds)),
                MetadataHeartbeat = TimeSpan.FromSeconds(heartbeatSeconds),
                MetadataLostAfter = TimeSpan.FromSeconds(lostAfterSeconds)
            };
        }

        private static string ReadEnvironmentVariable(string name, string fallback)
        {
            string value = Environment.GetEnvironmentVariable(name, EnvironmentVariableTarget.Machine);
            if (string.IsNullOrWhiteSpace(value))
            {
                value = Environment.GetEnvironmentVariable(name);
            }

            return string.IsNullOrWhiteSpace(value) ? fallback : value.Trim();
        }

        private static int ReadPositiveInt(string name, int fallback)
        {
            int parsed;
            return int.TryParse(ReadEnvironmentVariable(name, string.Empty), out parsed) && parsed > 0
                ? parsed
                : fallback;
        }
    }
}
