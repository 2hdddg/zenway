
function dump(o)
   if type(o) == 'table' then
      local s = '{ '
      for k,v in pairs(o) do
         if type(k) ~= 'number' then k = '"'..k..'"' end
         s = s .. '['..k..'] = ' .. dump(v) .. ','
      end
      return s .. '} '
   else
      return tostring(o)
   end
end

local COLOR_BLACK = '#1c1b19'
local COLOR_BLACK_BR = '#918175'
local COLOR_RED = '#ef2f27'
local COLOR_GREEN = '#519f50'

-- Config
zen.panels.left = {
  widgets = {
    {
      sources = {'time'},
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
            items = {
              {
                type = "box",
                markup = wmarkup,
                color = boxcolor,
                padding = {
                  left = 10,
                  right = 10,
                },
                radius = 8,
              },
              {
                type = "box",
                markup = amarkup,
                color = boxcolor,
                padding = {
                  left = 10,
                  right = 10,
                },
                radius = 8,
              },
            },
          }
          table.insert(workspaces, workspace)
        end
        return {
          type = "flex",
          direction = "column",
          items = workspaces,
        }
      end,
    },
  },
  screen_border_offset = 50,
}

zen.panels.right = {
  screen_border_offset = 50,
  widgets = {
    {
      sources = {'time', 'date'},
      render = function()
        local t = os.time()
        local markup = os.date(
            "<span font='digital-7' size='40pt' color='#1c1b19' rise='-3pt'>%H:%M</span><span size='15pt' color='#1c1b19'>\n%Y-%m-%d</span>")
        return {
          type = "box",
          markup = markup,
          color = COLOR_GREEN,
          radius = 10,
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
        sources = {'audio'},
        render = function()
          if zen.sources.audio.muted then
            return "<span size='15pt' color='#1c1b19' background='#918175'>Muted</span>"
          else
            return "<span size='15pt' color='#1c1b19' background='#918175'>Volume " .. zen.sources.audio.volume .. " </span>"
          end
        end,
    },
    {
        sources = {'power'},
        render = function()
          return "<span size='15pt' color='#1c1b19' background='#918175'>"  .. zen.sources.power.capacity .. "</span>"
        end,
    },
  },
}
