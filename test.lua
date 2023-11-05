
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

-- Config
zen.panels.left = {
  widgets = {
    {
      sources = {'workspace'},
      padding = {left = 10 },
      render = function(displayName)
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
            padding = {
              right = 1,
            },
            items = {
              {
                type = "box",
                markup = wmarkup,
                color = boxcolor,
                padding = {
                  top = 3,
                  left = 12,
                  right = 12,
                  bottom = 5,
                },
                radius = 15,
                border = {
                  width = 2,
                  color = boxcolor .. '10',
                },
              },
              {
                type = "box",
                markup = amarkup,
                color = boxcolor,
                padding = {
                  top = 3,
                  left = 10,
                  right = 10,
                  bottom = 5,
                },
                radius = 15,
                border = {
                  width = 2,
                  color = boxcolor .. '10',
                },
              },
            },
          }
          table.insert(workspaces, workspace)
        end
        return {
          type = "flex",
          direction = "column",
          padding = {
            left = 10,
            bottom = 1,
          },
          items = workspaces,
        }
      end,
    },
  },
}

zen.panels.right = {
  screen_border_offset = 50,
  widgets = {
    {
      sources = {'time', 'date'},
      padding = {
        bottom = 10,
        right = 10,
      },
      render = function()
        local t = os.time()
        local markup = os.date(
        "<span font='digital-7' size='40pt' color='#1c1b19' rise='-3pt'>%H:%M</span><span size='15pt' color='#1c1b19'>\n%Y-%m-%d</span>")
        return {
          type = "box",
          markup = markup,
          color = COLOR_BLUE_BR,
          radius = 15,
          border = {
            width = 2,
            color = COLOR_BLUE_BR .. '80',
          },
          padding = {
            left = 10,
            right = 10,
            top = 5,
            bottom = 5,
          },
        }
      end,
    },
    {
      sources = {'audio'},
      padding = {
        bottom = 10,
        right = 10,
      },
      render = function()
        local markup = ""
        local icon = ""
        local text = ""
        if zen.sources.audio.muted then
          icon = ""
          text = "Muted"
        else
          local volume = math.floor(zen.sources.audio.volume)
          icon = ""
          if volume > 10 then
            if volume < 50 then
              icon = ""
            else
              icon = ""
            end
          end
          text = "Volume " .. volume
        end
        markup = "<span size='30pt' color='" .. COLOR_BLACK .. "'>" .. icon .. "</span>" ..
        "<span size='15pt' rise='8pt' color='" .. COLOR_BLACK .. "'> " .. text .. "</span>"
        return {
          type = "box",
          markup = markup,
          color = COLOR_GREEN,
          radius = 15,
          border = {
            width = 2,
            color = COLOR_GREEN .. '80',
          },
          padding = {
            left = 10,
            right = 10,
            top = 5,
            bottom = 5,
          },
        }
      end,
    },
    {
      sources = {'power'},
      padding = {
        right = 10,
      },
      render = function()
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
        return {
          type = "box",
          markup = markup,
          color = color,
          radius = 15,
          border = {
            width = 2,
            color = color .. '80',
          },
          padding = {
            left = 10,
            right = 10,
            top = 5,
            bottom = 5,
          },
        }
      end,
    },
  },
}
