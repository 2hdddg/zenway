
local COLOR_BLACK = '#1c1b19'
local COLOR_RED = '#ef2f27'
local COLOR_GREEN = '#519f50'
local COLOR_YELLOW = '#fed06e'
local COLOR_BLUE = '#2c78bf'
local COLOR_MAGENTA = '#ff5c8f'
local COLOR_CYAN = '#2be4d0'
local COLOR_WHITE = '#fce8c3'

local COLOR_BLACK_BR = '#918175'
local COLOR_BLUE_BR = '#68a8e4'

local function get_icon_for_audio_type()
  local t = zen.sources.audio.port
  if t == "speaker" then
    return "󰓃"
  end
  if t == "headphones" then
    return ""
  end
  if t == "tv" then
    return ""
  end
  return ""
end

local function get_box(markup, color)
  return {
    type = "box",
    markup = markup,
    color = color,
    padding = {
      top = 3,
      left = 10,
      right = 10,
      bottom = 5,
    },
    radius = 15,
    border = {
      width = 2,
      color = color .. '10',
    },
  }
end

local function render_workspaces(displayName)
  local display = zen.sources.displays[displayName]
  if not display then
    return ""
  end
  local workspaces = {}
  for _, workspace in pairs(display.workspaces) do
    local wmarkup = "<span size='20pt' color='" .. COLOR_BLACK .. "'>" .. workspace.name .. "</span>"
    local amarkup = ""
    for _, app in pairs(workspace.applications) do
      if app.focus or (amarkup == "" and app.next) then
        amarkup = "<span size='20pt' color='" .. COLOR_BLACK .. "'>" .. app.name .. "</span>"
      end
    end
    boxcolor = COLOR_BLACK_BR
    if workspace.focus then
      boxcolor = COLOR_GREEN
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
  return get_box(markup, COLOR_BLUE_BR)
end

local function render_keyboard()
  icon = ""
  text = zen.sources.keyboard.layout
  markup = "<span size='30pt' color='" .. COLOR_BLACK .. "'>" .. icon .. "</span>" ..
  "<span size='12pt' rise='8pt' color='" .. COLOR_BLACK .. "'> " .. text .. "</span>"
  return get_box(markup, COLOR_CYAN)
end

local function render_audio()
  local markup = ""
  local icon = ""
  local icon2 = ""
  local text = ""
  color = COLOR_GREEN
  if zen.sources.audio.muted then
    icon = icon .. ""
    text = "Muted"
    color = COLOR_RED
  else
    icon = get_icon_for_audio_type()
    local volume = math.floor(zen.sources.audio.volume)
    icon2 = ""
    if volume > 10 then
      if volume < 50 then
        icon2 = ""
      else
        icon2 = ""
      end
    end
    text = "Volume " .. volume
  end
  markup =
    "<span size='30pt' color='" .. COLOR_BLACK .. "'>" .. icon .. "</span>" ..
    "<span size='15pt'> </span>" ..
    "<span size='30pt' color='" .. COLOR_BLACK .. "'>" .. icon2 .. "</span>" ..
    "<span size='15pt' rise='8pt' color='" .. COLOR_BLACK .. "'> " .. text .. "</span>"
  return get_box(markup, color)
end

local function render_power()
  local color = COLOR_GREEN
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
    if c > 90 then
      icon = ""
    elseif c > 80 then
      icon = ""
    elseif c > 50 then
      icon = ""
      color = COLOR_YELLOW
    elseif c > 20 then
      icon = ""
      color = COLOR_YELLOW
    else
      icon = ""
      color = COLOR_RED
    end
    text = "Battery " .. c .. "%"
  end
  local markup = "<span size='30pt' color='" .. COLOR_BLACK .. "'>" .. icon .. "</span>" ..
  "<span size='15pt' rise='8pt' color='" .. COLOR_BLACK .. "'> " .. text .. "</span>"
  return get_box(markup, color)
end

zen = {
  buffers = {
    num = 1,
    width = 1000,
    height = 1000,
  },
  sources = {},
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
