
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

local x = function()
    return {
        type = "flex",
        direction = "column",
        items = {
            "<span size='10pt' color='#1c1b19' background='#918175'>Testing 1</span>",
            "<span size='10pt' color='#1c1b19' background='#918175'>Test 2</span>",
            {
              type = "markup",
              markup = "<span size='10pt' color='#1c1b19' background='#918175'>Test 2</span>",
              background_color = "#111223",
              border_color = "",
              border_width = 2,
              border_radius = 2,
            },
            {
              type = "",
            },
        },
    }
end

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
          local wmarkup = "<span size='10pt' color='#1c1b19' background='#918175'>" .. workspace.name .. "</span>"
          local amarkup = ""
          for _, app in pairs(workspace.applications) do
            if app.focus then
              amarkup = "<span size='10pt' color='#1c1b19' background='#918175'>" .. app.name .. "</span>"
            end
            if amarkup == "" and app.next then
              amarkup = "<span size='10pt' color='#1c1b19' background='#918175'>" .. app.name .. "</span>"
            end
          end
          local workspace = {
            type = "flex",
            direction = "row",
            items = {
              wmarkup,
              amarkup,
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
          color = '#010101',
          radius = 10,
          border = {
            width = 2,
            color = '#ff0000ff',
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
