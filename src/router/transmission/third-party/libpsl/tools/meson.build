psl = executable('psl', 'psl.c',
  include_directories : configinc,
  c_args : ['-DHAVE_CONFIG_H'],
  link_language : link_language,
  dependencies : [libpsl_dep, networking_deps],
  install : true,
)

install_man('psl.1')
