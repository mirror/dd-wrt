addshare_lib = static_library(
  'addshare',
  'share_admin.c',
  'addshare.c',
  'share_admin.h',
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
  input: 'ksmbd.addshare.8.in',
  output: 'ksmbd.addshare.8',
  install_dir: get_option('mandir') / 'man8',
  configuration: in_data,
)

install_symlink(
  'ksmbd.addshare',
  install_dir: get_option('sbindir'),
  pointing_to: get_option('prefix') / get_option('libexecdir') / 'ksmbd.tools',
)
