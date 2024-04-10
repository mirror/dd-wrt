control_lib = static_library(
  'control',
  'control.c',
  include_directories: include_dirs,
  c_args: [
    '-DSYSCONFDIR="@0@"'.format(get_option('prefix') / get_option('sysconfdir')),
    '-DRUNSTATEDIR="@0@"'.format(runstatedir),
  ],
  dependencies: [
    glib_dep,
  ],
)

configure_file(
  input: 'ksmbd.control.8.in',
  output: 'ksmbd.control.8',
  install_dir: get_option('mandir') / 'man8',
  configuration: in_data,
)

install_symlink(
  'ksmbd.control',
  install_dir: get_option('sbindir'),
  pointing_to: get_option('prefix') / get_option('libexecdir') / 'ksmbd.tools',
)
