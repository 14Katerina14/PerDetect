using System;
using System.Collections.Generic;
using System.Drawing;
using System.Windows.Forms;
using SecureZone.XProtectPlugin.Background;
using VideoOS.Platform;
using VideoOS.Platform.Background;
using VideoOS.Platform.RuleAction;

namespace SecureZone.XProtectPlugin
{
    public class SecureZonePluginDefinition : PluginDefinition
    {
        internal static readonly Guid PluginId = new Guid("1b046cf8-213d-40ba-9242-87f670451bf0");
        internal static readonly Guid BackgroundPluginId = new Guid("8be4e450-ad96-419c-aed0-4625e794cb47");
        internal static readonly Guid ConfirmViolationActionId = new Guid("fc1810ab-475e-4975-af04-1eaf07b557ab");

        private readonly List<BackgroundPlugin> backgroundPlugins = new List<BackgroundPlugin>();
        private ActionManager actionManager;
        private Image icon;

        public override void Init()
        {
            icon = VideoOS.Platform.UI.Util.ImageList.Images[VideoOS.Platform.UI.Util.PluginIx];
            actionManager = new SecureZoneRuleActionManager();
            backgroundPlugins.Add(new SecureZoneEventServerBackgroundPlugin());

            EnvironmentManager.Instance.Log(false, "SecureZone", "SecureZone XProtect plugin initialized.");
        }

        public override void Close()
        {
            backgroundPlugins.Clear();
            actionManager = null;
        }

        public override Guid Id
        {
            get { return PluginId; }
        }

        public override Guid SharedNodeId
        {
            get { return Guid.Empty; }
        }

        public override string Name
        {
            get { return "SecureZone"; }
        }

        public override string SharedNodeName
        {
            get { return "SecureZone"; }
        }

        public override string Manufacturer
        {
            get { return "SecureZone"; }
        }

        public override string VersionString
        {
            get { return "1.0.0.0"; }
        }

        public override Image Icon
        {
            get { return icon; }
        }

        public override UserControl GenerateUserControl()
        {
            return null;
        }

        public override bool UserControlFillEntirePanel
        {
            get { return false; }
        }

        public override List<BackgroundPlugin> BackgroundPlugins
        {
            get { return backgroundPlugins; }
        }

        public override ActionManager ActionManager
        {
            get { return actionManager; }
        }
    }
}
