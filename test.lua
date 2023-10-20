
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
        return {
          type = "flex",
          items = {
            "<span size='10pt' color='#1c1b19' background='#918175'>Testing 1</span>",
            "<span size='10pt' color='#1c1b19' background='#918175'>Test 2</span>",
          },
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
        return os.date(
            "<span font='digital-7' size='40pt' color='#1c1b19' background='#918175' rise='-3pt'>%H:%M</span><span size='15pt' color='#1c1b19' background='#918175'>\n%Y-%m-%d</span>")
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

local xxx = function(displayName)
  local display = zen.sources.displays[displayName]
  if not display then
      return ""
  end
  local s = ""
  --table.insert(s, {})
  for w, workspace in pairs(display.workspaces) do
    s = s .. "  " .. workspace.name .. "\n"
    for a, app in pairs(workspace.applications) do
      s = s .. "    " .. app.name .. "\n"
    end
  end
  return "<span size='10pt' color='#1c1b19' background='#918175'>" .. s .. "</span>"
end
