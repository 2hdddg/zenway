scanner = find_program('wayland-scanner')
generate_c = generator(
  scanner,
  output: '@BASENAME@.c',
  arguments: ['private-code', '@INPUT@', '@OUTPUT@']
)
generate_h = generator(
  scanner,
  output: '@BASENAME@.h',
  arguments: ['client-header', '@INPUT@', '@OUTPUT@']
)
protocols = [
  'xdg-shell.xml',
  'wlr-layer-shell-unstable-v1.xml',
]
cfiles = []
hfiles = []
foreach p : protocols
  cfiles += generate_c.process(p)
  hfiles += generate_h.process(p)
endforeach
lib_protocol = static_library(
  'protocol',
  cfiles + hfiles,
  dependencies: [],
  include_directories: [],
)
internal_lib_protocol = declare_dependency(
  link_with: lib_protocol,
  sources: hfiles,
)
