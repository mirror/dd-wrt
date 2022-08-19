adduser = executable(
  'ksmbd.adduser',
  'md4_hash.c',
  'user_admin.c',
  'adduser.c',
  'md4_hash.h',
  'user_admin.h',
  dependencies: [glib_dep, netlink_dep],
  include_directories: tools_incdir,
  link_with: libksmbdtools,
  install: true,
  install_dir: get_option('sbindir'),
)
