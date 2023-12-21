# zenway
WIP sway bar alternative that is not really a bar but an overlay displayed on top of workspaces. The overlay is visible when Sway mod key is held down.

Screenshot shows Neovim and Firefox in background with zenway status shown above it.

![Alt text](/screenshots/beta2.png?raw=true "Screenshot of early beta with neovim in background")

# Features
* Configuration with lua.
* Dynamic layout similair to CSS flexbox but all in Lua (no html or css)
* Supports status for:
  - Sway workspaces including applications per workspace
  - Battery capacity
  - Audio (currently only PulseAudio), mute, volume 
  - Current time and date
  - Keyboard layout.
* Primitive click support

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

# Configuration

## Location
Searches for configuration file in:
* XDG_CONFIG_HOME/zenway/config.lua
* ~/.config/zenway/config.lua  

## Format
config.lua should return a Lua table that contains the static part
configuration properties:
```lua
return {
    panels = {
        {
            anchor = "left"
            widgets = {
                { sources = {'time', 'date'}, on_render = render_time },
                { sources = {'keyboard'}, on_render = render_keyboard, on_click = click_keyboard },
            },
            direction = "column",
        },
        {
            anchor = "right"
            ...
        },
}
```

The above configuratuion will display a panel with two widgets on the left side of the
screen. When zenway determines that any of the specified sources is dirty like timer expired in case of 'time'
or keyboard layout changed in case of 'keyboard' the specified on_render Lua function is invoked.
Zenway maintains state of all sources and makes that state accesible from Lua.

This is how the keyboard render function might look like:
```lua
local function render_keyboard()
    -- State of sources are kept in zen.<source>.<prop>
    local layout = zen.keyboard.layout
    -- Pango markup
    local markup = "<span size='20pt' color='#909090'>" .. layout .. "</span>"
    -- Simplified layout rendered by Zenway
    return {
        type = "box",
        markup = markup,
        color = "#1c1b19",
        radius = 5,
        border = { width = 2, color = '#10101080' },
    }
end
```

The widgets will be rendered with time aligned to the left with the keyboard rendered
below as specified by the direction = "column".

# How to build
TBD

