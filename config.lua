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
      if level >= v.level then
          return v
      end
  end
end


local function icon_markup(icon, color)
  return "<span size='30pt' color='" .. color .. "'>" .. icon .. "</span>"
end

local function space_markup()
  return "<span size='15pt'> </span>"
end

local function label_markup(label, color)
  return "<span size='15pt' rise='8pt' color='" .. color .. "'> " .. label .. "</span>"
end

local function get_box(markup, color)
  return {
    type = "box",
    markup = markup,
    color = color,
    padding = { top = 3, left = 10, right = 10, bottom = 5 },
    radius = 15,
    border = { width = 2, color = color .. '10' },
  }
end

local function render_workspaces(displayName)
  local display = zen.sources.displays[displayName]
  if not display then
    return ""
  end
  local workspaces = {}
  for _, workspace in pairs(display.workspaces) do
    local wmarkup = "<span size='20pt' color='" .. BLACK .. "'>" .. workspace.name .. "</span>"
    local amarkup = ""
    for _, app in pairs(workspace.applications) do
      if app.focus or (amarkup == "" and app.next) then
        amarkup = "<span size='20pt' color='" .. BLACK .. "'>" .. app.name .. "</span>"
      end
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
        get_box(wmarkup, boxcolor),
        get_box(amarkup, boxcolor),
      },
    }
    table.insert(workspaces, workspace)
  end
  return {
    type = "flex",
    direction = "column",
    padding = { left = 10, bottom = 1 },
    items = workspaces,
  }
end

local function render_time()
  local t = os.time()
  local markup = os.date(
  "<span font='digital-7' size='40pt' color='#1c1b19' rise='-3pt'>%H:%M</span><span size='15pt' color='#1c1b19'>\n%Y-%m-%d</span>")
  return get_box(markup, BLUE_BR)
end

local function render_keyboard()
  local label = "<span size='12pt' rise='8pt' color='" .. BLACK .. "'> " .. zen.sources.keyboard.layout .. "</span>"
  return get_box(icon_markup("", BLACK) .. label, CYAN)
end

local function render_audio()
  if zen.sources.audio.muted then
    return get_box(
        icon_markup(audio_port_icons["muted"], BLACK) .. label_markup("Muted", BLACK),
        RED)
  end
  local markup = icon_markup(audio_port_icons[zen.sources.audio.port], BLACK)
  local volume = math.floor(zen.sources.audio.volume)
  local level = find_level(audio_levels, volume)
  markup = markup .. icon_markup(level.icon, BLACK)
  markup = markup .. label_markup("Volume " .. volume, BLACK)
  return get_box(markup, GREEN)
end

local function render_power()
  local color = GREEN
  local icon = ""
  local text = ""
  if zen.sources.power.isCharging then
    icon = ""
    text = "Charging"
  elseif zen.sources.power.isPluggedIn then
    icon = ""
    text = "Fully charged"
  else
    local c = zen.sources.power.capacity
    local level = find_level(power_levels, c)
    icon = level.icon
    color = level.color
    text = "Battery " .. c .. "%"
  end
  return get_box(icon_markup(icon, BLACK) .. label_markup(text, BLACK), color)
end

zen = {
  buffers = {
    num = 1,
    width = 1000,
    height = 1000,
  },
  panels = {
    left = {
      widgets = {
        {
          sources = {'workspace'},
          padding = {left = 10 },
          render = render_workspaces,
        },
      },
    },
    right = {
      widgets = {
        {
          sources = {'time', 'date'},
          padding = { bottom = 10, right = 10 },
          render = render_time,
        },
        {
          sources = {'keyboard'},
          padding = { bottom = 10, right = 10 },
          render = render_keyboard,
        },
        {
          sources = {'audio'},
          padding = { bottom = 10, right = 10 },
          render = render_audio,
        },
        {
          sources = {'power'},
          padding = { right = 10 },
          render = render_power,
        },
      },
    },
  },
}
