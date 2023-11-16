-- Normal
local BLACK = '#1c1b19'
local RED = '#ef2f27'
local GREEN = '#519f50'
local YELLOW = '#fed06e'
local BLUE = '#2c78bf'
local MAGENTA = '#ff5c8f'
local CYAN = '#2be4d0'
local WHITE = '#fce8c3'
-- Bright
local BLACK_BR = '#918175'
local BLUE_BR = '#68a8e4'

local TEXT_COLOR = BLACK
local TEXT_SIZE = "15pt"
local TEXT_RISE = "8pt"  -- To align with icons
local SMALL_TEXT_SIZE = "11pt"
local SMALL_TEXT_RISE = "6pt"
local ICON_SIZE = "30pt"
local SMALL_ICON_SIZE = "22pt"

local empty_string_meta = {
    __index = function() return "" end,
}

local audio_port_icons = {
    speaker = "󰓃",
    headphones = "",
    tv = "",
    muted = "",
}
setmetatable(audio_port_icons, empty_string_meta)

local audio_levels = {
    { level = 50, icon = "" },
    { level = 10, icon = ""},
    { level = 0, icon = ""},
}

local power_levels  = {
    { level = 90, icon = "", color = GREEN },
    { level = 80, icon = "", color = GREEN },
    { level = 50, icon = "", color = YELLOW },
    { level = 20, icon = "", color = YELLOW },
    { level = 0, icon = "", color = RED },
}

local function find_level(table, level)
    for i, v in ipairs(table) do
        if level >= v.level then return v end
    end
end

local function icon(p)
    local color = p["color"] or TEXT_COLOR
    local icon = p["icon"]
    local size = p["size"] or ICON_SIZE
    return "<span size='" .. size .. "' color='" .. color .. "'>" .. icon .. "</span>"
end

local function label(p)
    local color = p["color"] or TEXT_COLOR
    local label = p["label"]
    local size = p["size"] or TEXT_SIZE
    local rise = p["rise"] or TEXT_RISE
    return "<span size='" .. size .. "' rise='" .. rise .. "' color='" .. color .. "'>" .. label .. "</span>"
end

local function box(markup, color)
    return {
        type = "box",
        markup = markup,
        color = color,
        padding = { top = 5, left = 10, right = 10, bottom = 5 },
        radius = 15,
        border = { width = 2, color = color .. '10' },
    }
end

local function render_workspaces(displayName)
    local display = zen.displays[displayName]
    if not display then return "" end
    local workspaces = {}
    for _, workspace in pairs(display.workspaces) do
        local app_name = ""
        for _, app in pairs(workspace.applications) do
            if app.focus then app_name = app.name end
        end
        boxcolor = BLACK_BR
        if workspace.focus then
            boxcolor = GREEN
        end
        local workspace = {
            type = "flex",
            direction = "row",
            padding = { right = 1 },
            items = {
                box(label{label=workspace.name}, boxcolor),
                box(label{label=app_name}, boxcolor),
            },
        }
        table.insert(workspaces, workspace)
    end
    return { type = "flex", direction = "column", padding = { left = 10, bottom = 10 }, items = workspaces }
end

local function render_time()
    local t = os.time()
    local markup = os.date("<span font='digital-7' size='40pt' color='" .. BLACK .. "' rise='-3pt'>%H:%M</span><span size='15pt' color='" .. BLACK .. "'>\n%Y-%m-%d</span>")
    return box(markup, BLUE_BR)
end

local function render_keyboard()
    return box(icon{icon="", size=SMALL_ICON_SIZE} .. label{label=" " .. zen.keyboard.layout, size=SMALL_TEXT_SIZE, rise=SMALL_TEXT_RISE}, CYAN)
end

local function render_audio()
    if zen.audio.muted then
        return box(icon{icon=audio_port_icons["muted"]} .. label{label=" Muted"}, RED)
    end
    local markup = icon{icon=audio_port_icons[zen.audio.port]}
    local volume = math.floor(zen.audio.volume)
    local level = find_level(audio_levels, volume)
    markup = markup .. icon{icon=level.icon}
    markup = markup .. label{label=" Volume " .. volume}
    return box(markup, GREEN)
end

local function render_power()
    if zen.power.isCharging then return box(icon{icon = ""} .. label{label = " Charging"}, GREEN) end
    if zen.power.isPluggedIn then return box(icon{icon = ""} .. label{label = " Fully charged"}, GREEN) end
    local c = zen.power.capacity
    local level = find_level(power_levels, c)
    return box(icon{icon = level.icon} .. label{label = " Battery " .. c .. "%"}, level.color)
end

local function render_networks()
    local up = nil
    for _, network in pairs(zen.networks) do
        if up == nil and network.up then
            up = network
        end
    end
    if up then
        return box(icon{icon = "󰣐"} .. label{label = " Network " .. up.interface .. ": " .. up.address}, GREEN)
    else
        return box(icon{icon = "󰋔"} .. label{label = "Network down!"}, RED)
    end
end

return {
    buffers = {
        num = 1,
        width = 1000,
        height = 1000,
    },
    panels = {
        {
            widgets = {
                { sources = {'workspace'}, padding = {left = 10 }, render = render_workspaces },
            },
        },
        {
            widgets = {
                { sources = {'time', 'date'}, padding = { bottom = 10, right = 10 }, render = render_time },
                { sources = {'keyboard'}, padding = { bottom = 10, right = 10 }, render = render_keyboard },
                { sources = {'audio'}, padding = { bottom = 10, right = 10 }, render = render_audio },
                { sources = {'power'}, padding = { bottom = 10, right = 10 }, render = render_power },
                { sources = {'networks'}, padding = { right = 10 }, render = render_networks },
            },
        },
    },
}
