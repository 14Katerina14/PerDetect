using VideoOS.Platform;

namespace SecureZone.XProtectPlugin.Background
{
    internal sealed class MipDecisionBridgeLogger : IDecisionBridgeLogger
    {
        public void Info(string message)
        {
            EnvironmentManager.Instance.Log(false, "SecureZone.DecisionBridge", message);
        }

        public void Error(string message)
        {
            EnvironmentManager.Instance.Log(true, "SecureZone.DecisionBridge", message);
        }
    }
}
