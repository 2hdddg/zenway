src += files(
  'Buffer.cpp',
  'Configuration.cpp',
  'Draw.cpp',
  'main.cpp',
  'MainLoop.cpp',
  'Manager.cpp',
  'Output.cpp',
  'Registry.cpp',
  'ScriptContext.cpp',
  'Seat.cpp',
  'ShellSurface.cpp',
  'util.cpp',
)
deps += dependency('wayland-client')
deps += dependency('wayland-protocols')
deps += dependency('cairo')
deps += dependency('lua')
deps += meson.get_compiler('c').find_library('m')
deps += dependency('xkbcommon')
deps += dependency('pango')
deps += dependency('pangocairo')
deps += subproject('spdlog', default_options: 'tests=false').get_variable('spdlog_dep')
deps += subproject('nlohmann_json').get_variable('nlohmann_json_dep')
deps += internal_lib_protocol
subdir('Sources')
subdir('Compositors/Sway')
