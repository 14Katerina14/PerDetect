using System;
using VideoOS.Platform.Data;

namespace SecureZone.XProtectPlugin.Background
{
    internal sealed class LineCrossingEventSnapshot
    {
        public string EventId { get; set; }
        public string EventName { get; set; }
        public string SourceName { get; set; }
        public DateTime ReceivedAtUtc { get; set; }
        public EventSource Source { get; set; }
    }
}
