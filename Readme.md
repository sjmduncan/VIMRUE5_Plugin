# VIMRUE5_Plugin

Make the following changes to the config files to get telepresence working:

**`DefaultEngine.ini`**

```
[Voice] 
bEnabled=true

[OnlineSubsystem]
DefaultPlatformService=Steam
bHasVoiceEnabled=true

[OnlineSubsystemSteam]
bEnabled=true
SteamDevAppId=480
bUseSteamNetworking=true

[/Script/OnlineSubsystemUtils.IpNetDriver]
InitialConnectTimeout=120.0
MaxClientRate=2000000
MaxInternetClientRate=2000000

[/Script/Engine.GameEngine]
+NetDriverDefinitions=(DefName="GameNetDriver",DriverClassName="OnlineSubsystemSteam.SteamNetDriver",DriverClassNameFallback="OnlineSubsystemUtils.IpNetDriver")

[/Script/OnlineSubsystemSteam.SteamNetDriver]
NetConnectionClassName="OnlineSubsystemSteam.SteamNetConnection"

[SystemSettings]
voice.SilenceDetectionThreshold=0.01
voice.MicNoiseGateThreshold=0.01
```

**`DefaultGame.ini`**

```
[/script/Engine.GameSession]
bRequiresPushToTalk=false
```