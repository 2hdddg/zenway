
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

-- Config
zen.panels.left = {
  widgets = {
    {
      sources = {'time'},
      render = function()
        local s = ""
        for displayName, display in pairs(zen.sources.displays) do
          s = s .. displayName .. "\n"
          for w, workspace in pairs(display.workspaces) do
            s = s .. "  " .. workspace.name .. "\n"
            for a, app in pairs(workspace.applications) do
              s = s .. "    " .. app.name .. "\n"
            end
          end
        end
        return "<span size='10pt' color='#1c1b19' background='#918175'>" .. s .. "</span>"
      end,
    },
  },
}

zen.panels.right = {
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
