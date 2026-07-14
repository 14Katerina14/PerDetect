# SecureZone XProtect Plugin

This project is a standalone Milestone XProtect MIP plugin for the Event Server.
It is intentionally separated from the SecureZone backend modules.

## Purpose

The plugin belongs on the company XProtect machine, not on the development
laptop. XProtect Event Server loads it and uses it as part of the XProtect
rules/event flow.

The current plugin does two things:

- listens for XProtect Event Server events and logs WiseAI LineCrossing events
- exposes a custom rule action named `SecureZone: confirm violation`

The custom rule action raises a new XProtect event:

```text
Type: SecureZoneViolationConfirmed
Name: SecureZone violation confirmed
Class: SecureZone
```

This event can then be used by XProtect rules to show an alarm in Smart Client,
notify operators, or start an audio/speaker action.

## Expected Flow

```text
Hanwha/WiseAI camera event rule
        |
        v
XProtect Event Server receives WiseAI LineCrossing event
        |
        v
XProtect rule runs SecureZone custom action
        |
        v
SecureZoneViolationConfirmed event is raised
        |
        v
XProtect alarm definition / Smart Client / speaker rule
```

## Build

Open `SecureZone.XProtectPlugin.csproj` in Visual Studio on a machine with the
Milestone MIP SDK/.NET dependencies available, or build it with MSBuild.

The project references:

```text
MilestoneSystems.VideoOS.Platform
```

## Install On The Company XProtect Machine

1. Build the project in `Release`.
2. Create a plugin folder on the XProtect/Event Server machine, for example:

   ```text
   C:\Program Files\Milestone\MIPPlugins\SecureZone.XProtectPlugin
   ```

3. Copy these files from the build output:

   ```text
   SecureZone.XProtectPlugin.dll
   plugin.def
   ```

4. Restart the XProtect Event Server service.
5. Open Management Client.
6. Create or edit a rule and look for the action:

   ```text
   SecureZone: confirm violation
   ```

## Current MVP Notes

The plugin does not connect to the SecureZone backend yet. For the MVP, it is
only the XProtect-side rule/action adapter. Backend business validation can be
added later through HTTP, local IPC, or another agreed integration channel.
