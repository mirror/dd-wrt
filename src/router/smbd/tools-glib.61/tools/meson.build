ksmbd_tools_files = [
  'management/tree_conn.c',
  'management/user.c',
  'management/share.c',
  'management/session.c',
  'config_parser.c',
  'tools.c',
]

if krb5_dep.found()
  ksmbd_tools_files += [
    'management/spnego.c',
    'asn1.c',
    'management/spnego_krb5.c',
  ]
endif

executable(
  'ksmbd.tools',
  ksmbd_tools_files,
  include_directories: include_dirs,
  c_args: [
    '-DSYSCONFDIR="@0@"'.format(get_option('prefix') / get_option('sysconfdir')),
    '-DRUNSTATEDIR="@0@"'.format(runstatedir),
  ],
  dependencies: [
    glib_dep,
    krb5_dep,
    asn1_lib,
    pthread_lib,
  ],
  link_with: [
    addshare_lib,
    adduser_lib,
    control_lib,
    mountd_lib,
  ],
  install: true,
  install_dir: get_option('libexecdir'),
)
