using System;
using System.Collections.Generic;
using VideoOS.Platform;
using VideoOS.Platform.Background;
using VideoOS.Platform.Data;
using VideoOS.Platform.Messaging;

namespace SecureZone.XProtectPlugin.Background
{
    public class SecureZoneEventServerBackgroundPlugin : BackgroundPlugin
    {
        private object eventReceiver;

        public override Guid Id
        {
            get { return SecureZonePluginDefinition.BackgroundPluginId; }
        }

        public override string Name
        {
            get { return "SecureZone Event Server Plugin"; }
        }

        public override void Init()
        {
            eventReceiver = EnvironmentManager.Instance.RegisterReceiver(
                NewEventHandler,
                new MessageIdFilter(MessageId.Server.NewEventIndication)
            );

            EnvironmentManager.Instance.Log(false, "SecureZone", "Listening for XProtect Event Server events.");
        }

        public override void Close()
        {
            if (eventReceiver != null)
            {
                EnvironmentManager.Instance.UnRegisterReceiver(eventReceiver);
                eventReceiver = null;
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
            string sourceName = header.Source != null ? header.Source.Name : string.Empty;

            if (!IsWiseAiLineCrossing(eventName, eventType, eventMessage))
            {
                return null;
            }

            EnvironmentManager.Instance.Log(
                false,
                "SecureZone.LineCrossing",
                "WiseAI LineCrossing event received. " +
                "Name='" + eventName + "', Type='" + eventType + "', Source='" + sourceName +
                "', Message='" + eventMessage + "'."
            );

            return null;
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
            return value != null &&
                   value.IndexOf(token, StringComparison.OrdinalIgnoreCase) >= 0;
        }
    }
}
