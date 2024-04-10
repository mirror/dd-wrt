mountd_lib = static_library(
  'mountd',
  'worker.c',
  'ipc.c',
  'rpc.c',
  'rpc_srvsvc.c',
  'rpc_wkssvc.c',
  'mountd.c',
  'smbacl.c',
  'rpc_samr.c',
  'rpc_lsarpc.c',
  include_directories: include_dirs,
  c_args: [
    '-DSYSCONFDIR="@0@"'.format(get_option('prefix') / get_option('sysconfdir')),
    '-DRUNSTATEDIR="@0@"'.format(runstatedir),
  ],
  dependencies: [
    glib_dep,
    libnl_dep,
  ],
)

configure_file(
  input: 'ksmbd.mountd.8.in',
  output: 'ksmbd.mountd.8',
  install_dir: get_option('mandir') / 'man8',
  configuration: in_data,
)

install_symlink(
  'ksmbd.mountd',
  install_dir: get_option('sbindir'),
  pointing_to: get_option('prefix') / get_option('libexecdir') / 'ksmbd.tools',
)
