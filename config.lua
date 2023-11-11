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

local DEFAULT_TEXT_COLOR = BLACK

local NORMAL_TEXT_SIZE = "15pt"
local NORMAL_TEXT_RISE = "8pt"  -- To align with icons
local SMALL_TEXT_SIZE = "11pt"
local SMALL_TEXT_RISE = "6pt"

local NORMAL_ICON_SIZE = "30pt"
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
      if level >= v.level then
          return v
      end
  end
end


local function icon_markup(p)
  local color = p["color"] or DEFAULT_TEXT_COLOR
  local icon = p["icon"]
  local size = p["size"] or NORMAL_ICON_SIZE
  return "<span size='" .. size .. "' color='" .. color .. "'>" .. icon .. "</span>"
end

local function space_markup()
  return "<span size='".. NORMAL_TEXT_SIZE .. "'> </span>"
end

local function label_markup(p)
  local color = p["color"] or DEFAULT_TEXT_COLOR
  local label = p["label"]
  local size = p["size"] or NORMAL_TEXT_SIZE
  local rise = p["rise"] or NORMAL_TEXT_RISE
  return "<span size='" .. size .. "' rise='" .. rise .. "' color='" .. color .. "'>" .. label .. "</span>"
end

local function get_box(markup, color)
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
  local display = zen.sources.displays[displayName]
  if not display then
    return ""
  end
  local workspaces = {}
  for _, workspace in pairs(display.workspaces) do
    local app_name = ""
    for _, app in pairs(workspace.applications) do
      if app.focus or (app_name == "" and app.next) then
        app_name = app.name
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
        get_box(label_markup{label=workspace.name}, boxcolor),
        get_box(label_markup{label=app_name}, boxcolor),
      },
    }
    table.insert(workspaces, workspace)
  end
  return {
    type = "flex",
    direction = "column",
    padding = { left = 10, bottom = 10 },
    items = workspaces,
  }
end

local function render_time()
  local t = os.time()
  local markup = os.date(
  "<span font='digital-7' size='40pt' color='" .. BLACK .. "' rise='-3pt'>%H:%M</span><span size='15pt' color='" .. BLACK .. "'>\n%Y-%m-%d</span>")
  return get_box(markup, BLUE_BR)
end

local function render_keyboard()
  return get_box(
    icon_markup{icon="", size=SMALL_ICON_SIZE} .. label_markup{label=" " .. zen.sources.keyboard.layout, size=SMALL_TEXT_SIZE, rise=SMALL_TEXT_RISE},
    CYAN)
end

local function render_audio()
  if zen.sources.audio.muted then
    return get_box(
        icon_markup{icon=audio_port_icons["muted"]} .. label_markup{label=" Muted"},
        RED)
  end
  local markup = icon_markup{icon=audio_port_icons[zen.sources.audio.port]}
  local volume = math.floor(zen.sources.audio.volume)
  local level = find_level(audio_levels, volume)
  markup = markup .. icon_markup{icon=level.icon}
  markup = markup .. label_markup{label=" Volume " .. volume}
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
    text = " Battery " .. c .. "%"
  end
  return get_box(icon_markup{icon=icon} .. label_markup{label=text}, color)
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
