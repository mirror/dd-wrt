project('ksmbsd-tools', 'c',
  version: run_command(find_program('awk'), '''/define KSMBD_TOOLS_VERSION / \
    { gsub(/"/,"",$3); printf "%s", $3 }''', 'include/version.h', check: true).stdout(),
  default_options: 'c_std=gnu99')

tools_incdir = include_directories(['include', '.'])

glib_dep = dependency('glib-2.0')
netlink_dep = dependency('libnl-genl-3.0')
krb5_dep = dependency(get_option('krb5_name'), required: get_option('krb5'))
asn1_lib = []

cc = meson.get_compiler('c')

cdata = configuration_data()
add_project_arguments('-DHAVE_CONFIG_H', language: 'c')
if krb5_dep.found()
  cdata.set('CONFIG_KRB5', krb5_dep.found())
  cdata.set('HAVE_KRB5_KEYBLOCK_KEYVALUE', cc.has_member('krb5_keyblock', 'keyvalue', prefix: '#include <krb5.h>', dependencies: krb5_dep))
  cdata.set('HAVE_KRB5_AUTHENTICATOR_CLIENT', cc.has_member('krb5_authenticator', 'client', prefix: '#include <krb5.h>', dependencies: krb5_dep))
  cdata.set('HAVE_KRB5_AUTH_CON_GETRECVSUBKEY', cc.has_function('krb5_auth_con_getrecvsubkey', dependencies: krb5_dep))
  cdata.set('HAVE_KRB5_AUTH_CON_GETAUTHENTICATOR_DOUBLE_POINTER', cc.compiles('''#include <krb5.h>
    krb5_error_code krb5_auth_con_getauthenticator(krb5_context, krb5_auth_context, krb5_authenticator**);''', dependencies: krb5_dep))
  if not cdata.get('HAVE_KRB5_AUTHENTICATOR_CLIENT')
    asn1_lib = cc.find_library('asn1', dirs: krb5_dep.get_pkgconfig_variable('libdir'))
  endif
endif
cfile = configure_file(
  output: 'config.h',
  configuration: cdata,
)

subdir('lib')
subdir('addshare')
subdir('adduser')
subdir('control')
subdir('mountd')
