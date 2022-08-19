addshare = executable(
  'ksmbd.addshare',
  'share_admin.c',
  'addshare.c',
  'share_admin.h',
  dependencies: [glib_dep, netlink_dep],
  include_directories: tools_incdir,
  link_with: libksmbdtools,
  install: true,
  install_dir: get_option('sbindir'),
)
