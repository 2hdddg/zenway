project(
  'zenway', 'cpp', 'c',
  default_options: [
    'cpp_std=c++20',
    'default_library=static',
    'warning_level=3',
    'b_sanitize=address',
  ],
)

compiler = meson.get_compiler('cpp')
cpp_args = []
add_global_arguments(cpp_args, language:'cpp')

wayland_client = dependency('wayland-client')
wayland_protocols = dependency('wayland-protocols')
cairo = dependency('cairo')
lua = dependency('lua')
m_dep = meson.get_compiler('c').find_library('m')
xkbcommon = dependency('xkbcommon')
deps = [wayland_client, wayland_protocols, xkbcommon, cairo, dependency('pango'), dependency('pangocairo'), dependency('libpulse'), lua, m_dep]
deps += subproject('spdlog').get_variable('spdlog_dep')
deps += subproject('nlohmann_json').get_variable('nlohmann_json_dep')

src = files(
  'src/Buffer.cpp',
  'src/Configuration.cpp',
  'src/DateTimeSources.cpp',
  'src/main.cpp',
  'src/MainLoop.cpp',
  'src/Manager.cpp',
  'src/NetworkSource.cpp',
  'src/Output.cpp',
  'src/Outputs.cpp',
  'src/Panel.cpp',
  'src/PowerSource.cpp',
  'src/PulseAudioSource.cpp',
  'src/Registry.cpp',
  'src/Roots.cpp',
  'src/ScriptContext.cpp',
  'src/Seat.cpp',
  'src/ShellSurface.cpp',
  'src/Sources.cpp',
  'src/SwayCompositor.cpp',
  'src/Theme.cpp',
)
zenway_src = src

subdir('protocols')
deps += internal_lib_protocol

executable(
  'zenway',
  src,
  dependencies: deps,
  include_directories: ['external'],
)
