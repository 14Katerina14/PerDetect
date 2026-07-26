using System;

namespace SecureZone.XProtectPlugin.Background
{
    internal interface IDecisionEventPublisher
    {
        void Publish(LineCrossingEventSnapshot sourceEvent, SecureZoneDecisionResponse decision);
        void PublishCleared(LineCrossingEventSnapshot sourceEvent, SecureZoneDecisionResponse decision);
    }

    internal interface IDecisionBridgeLogger
    {
        void Info(string message);
        void Error(string message);
    }

    internal sealed class DecisionBridgeProcessor
    {
        private readonly IDecisionEventPublisher publisher;
        private readonly IDecisionBridgeLogger logger;

        public DecisionBridgeProcessor(IDecisionEventPublisher publisher, IDecisionBridgeLogger logger)
        {
            this.publisher = publisher ?? throw new ArgumentNullException("publisher");
            this.logger = logger ?? throw new ArgumentNullException("logger");
        }

        public void Process(LineCrossingEventSnapshot snapshot, SecureZoneDecisionResponse decision)
        {
            if (snapshot == null) throw new ArgumentNullException("snapshot");
            if (decision == null) throw new ArgumentNullException("decision");

            if (decision.duplicate)
            {
                logger.Info("Duplicate backend decision ignored for event '" + snapshot.EventId + "'.");
                return;
            }

            if (decision.accepted && IsDecision(decision, "violation"))
            {
                if (!HasSource(snapshot, "confirmed violation")) return;
                publisher.Publish(snapshot, decision);
                return;
            }

            if (decision.accepted && IsDecision(decision, "cleared"))
            {
                if (!HasSource(snapshot, "cleared violation")) return;
                publisher.PublishCleared(snapshot, decision);
                return;
            }

            logger.Info(
                "No violation event raised for event '" + snapshot.EventId +
                "'. Backend decision='" + (decision.decision ?? "none") +
                "', status='" + (decision.status ?? "unknown") + "'."
            );
        }

        public void BackendFailure(LineCrossingEventSnapshot snapshot, Exception exception)
        {
            string eventId = snapshot == null ? "unknown" : snapshot.EventId;
            string reason = exception == null ? "Unknown backend error." : exception.Message;
            logger.Error(
                "SecureZone API decision failed for event '" + eventId +
                "'. No confirmed violation event was raised. " + reason
            );
        }

        private bool HasSource(LineCrossingEventSnapshot snapshot, string decisionName)
        {
            if (snapshot.Source != null) return true;

            logger.Error(
                "Backend returned " + decisionName +
                ", but the XProtect event has no source. Event '" +
                snapshot.EventId + "' was not raised."
            );
            return false;
        }

        private static bool IsDecision(SecureZoneDecisionResponse decision, string expected)
        {
            return string.Equals(decision.decision, expected, StringComparison.OrdinalIgnoreCase);
        }
    }
}
