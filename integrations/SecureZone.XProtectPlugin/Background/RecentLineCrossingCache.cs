using System;
using System.Collections.Generic;
using System.Linq;
using SecureZone.XProtectPlugin.Metadata;

namespace SecureZone.XProtectPlugin.Background
{
    internal sealed class RecentLineCrossingCache
    {
        private readonly object sync = new object();
        private readonly List<RawLineCrossing> entries = new List<RawLineCrossing>();

        public void Add(RawLineCrossing crossing)
        {
            lock (sync)
            {
                entries.RemoveAll(x => x.OccurredAtUtc < DateTime.UtcNow.AddSeconds(-10));
                entries.Add(crossing);
            }
        }

        public RawLineCrossing TakeClosest(DateTime eventTimeUtc, string cameraId)
        {
            lock (sync)
            {
                RawLineCrossing match = entries
                    .Where(x => (string.IsNullOrEmpty(cameraId) || x.CameraId == cameraId) &&
                                Math.Abs((x.OccurredAtUtc - eventTimeUtc).TotalSeconds) <= 5)
                    .OrderBy(x => Math.Abs((x.OccurredAtUtc - eventTimeUtc).TotalMilliseconds))
                    .FirstOrDefault();
                if (match != null) entries.Remove(match);
                return match;
            }
        }
    }
}
