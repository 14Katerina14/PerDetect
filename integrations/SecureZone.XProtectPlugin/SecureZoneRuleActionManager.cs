using System;
using System.Collections.ObjectModel;
using VideoOS.Platform;
using VideoOS.Platform.Data;
using VideoOS.Platform.Messaging;
using VideoOS.Platform.RuleAction;

namespace SecureZone.XProtectPlugin
{
    public class SecureZoneRuleActionManager : ActionManager
    {
        public override Collection<ActionDefinition> GetActionDefinitions()
        {
            return new Collection<ActionDefinition>
            {
                new ActionDefinition
                {
                    Id = SecureZonePluginDefinition.ConfirmViolationActionId,
                    Name = "SecureZone: confirm violation",
                    SelectionText = "Raise SecureZone violation event for <camera>",
                    DescriptionText = "Raise SecureZone violation event for {0}",
                    ActionItemKind = new ActionElement
                    {
                        DefaultText = "Camera",
                        ItemKinds = new Collection<Guid> { Kind.Camera }
                    }
                }
            };
        }

        public override void ExecuteAction(Guid actionId, Collection<FQID> actionItems, BaseEvent sourceEvent)
        {
            if (actionId != SecureZonePluginDefinition.ConfirmViolationActionId)
            {
                return;
            }

            EventSource eventSource = ResolveEventSource(actionItems, sourceEvent);
            if (eventSource == null || eventSource.FQID == null)
            {
                EnvironmentManager.Instance.Log(true, "SecureZone.RuleAction", "Cannot raise SecureZone event without a source item.");
                return;
            }

            AnalyticsEvent secureZoneEvent = new AnalyticsEvent
            {
                EventHeader = new EventHeader
                {
                    ID = Guid.NewGuid(),
                    Class = "SecureZone",
                    Type = "SecureZoneViolationConfirmed",
                    Timestamp = DateTime.Now,
                    Name = "SecureZone violation confirmed",
                    Message = BuildMessage(sourceEvent),
                    Source = eventSource,
                    Priority = 2,
                    PriorityName = "High"
                }
            };

            EnvironmentManager.Instance.SendMessage(
                new Message(MessageId.Server.NewEventCommand) { Data = secureZoneEvent }
            );

            EnvironmentManager.Instance.Log(
                false,
                "SecureZone.RuleAction",
                "Raised SecureZoneViolationConfirmed event for source '" + eventSource.Name + "'."
            );
        }

        private static EventSource ResolveEventSource(Collection<FQID> actionItems, BaseEvent sourceEvent)
        {
            if (actionItems != null && actionItems.Count > 0)
            {
                Item item = Configuration.Instance.GetItem(actionItems[0]);
                if (item != null)
                {
                    return new EventSource
                    {
                        FQID = item.FQID,
                        Name = item.Name
                    };
                }
            }

            if (sourceEvent != null &&
                sourceEvent.EventHeader != null &&
                sourceEvent.EventHeader.Source != null)
            {
                return sourceEvent.EventHeader.Source;
            }

            return null;
        }

        private static string BuildMessage(BaseEvent sourceEvent)
        {
            if (sourceEvent == null || sourceEvent.EventHeader == null)
            {
                return "SecureZone violation confirmed.";
            }

            EventHeader header = sourceEvent.EventHeader;
            return "SecureZone violation confirmed from source event '" +
                   (header.Name ?? "unknown") +
                   "' at " +
                   header.Timestamp.ToString("O") +
                   ".";
        }
    }
}
