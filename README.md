# zenway
WIP sway bar alternative that is not really a bar but an overlay displayed on top of workspaces. The overlay is visible when Sway mod key is held down.

Screenshot shows Neovim and Firefox in background with zenway status shown above it.

![Alt text](/screenshots/beta2.png?raw=true "Screenshot of early beta with neovim in background")

# Features
* Configuration with lua.
* Dynamic layout similair to CSS flexbox
* Supports status for Sway workspaces, battery, audio (only PulseAudio), current time and keyboard layout.

# Installation
In Sway config add entry to start Zenway:
```
exec zenway
```
In Sway config bar config:
```
bar {
  mode hide
  modifier Mod1
  swaybar_command whatever
}
```
# How to build
TBD
