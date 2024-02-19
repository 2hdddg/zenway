project(
  'zenway', 'cpp', 'c',
  default_options: [
    'cpp_std=c++20',
    'default_library=static',
    'warning_level=3',
  ],
)

compiler = meson.get_compiler('cpp')
cpp_args = []
add_global_arguments(cpp_args, language:'cpp')

deps = []
src = files()
subdir('protocols')
subdir('zen')
executable(
  'zenway',
  src,
  dependencies: deps,
  include_directories: ['../external'],
)
