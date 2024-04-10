project(
  'ksmbd-tools',
  'c',
  version: run_command(
    '/bin/sh',
    '-c',
    '''
exec awk '/define KSMBD_TOOLS_VERSION / { gsub(/"/,"",$3); printf "%s", $3 }' include/version.h
    ''',
    check: true,
  ).stdout(),
  default_options: 'c_std=gnu99',
  meson_version: '>= 0.61.5',
)

include_dirs = include_directories(
  '.',
  'include',
)

glib_dep = dependency(
  'glib-2.0',
  version: '>= 2.44',
)
libnl_dep = dependency(
  'libnl-genl-3.0',
  version: '>= 3.0',
)
systemd_dep = dependency(
  'systemd',
  required: false,
  version: '>= 245',
)
krb5_dep = dependency(
  get_option('krb5_name'),
  required: get_option('krb5'),
)
asn1_lib = []

config_h_data = configuration_data()
cc = meson.get_compiler('c')

pthread_lib = cc.find_library(
  'pthread',
)

if krb5_dep.found()
  config_h_data.set(
    'CONFIG_KRB5',
    krb5_dep.found(),
  )
  config_h_data.set(
    'HAVE_KRB5_AUTH_CON_GETRECVSUBKEY',
    cc.has_function(
      'krb5_auth_con_getrecvsubkey',
      dependencies: krb5_dep,
    ),
  )
  config_h_data.set(
    'HAVE_KRB5_KEYBLOCK_KEYVALUE',
    cc.has_member(
      'krb5_keyblock',
      'keyvalue',
      prefix: '#include <krb5.h>',
      dependencies: krb5_dep,
    ),
  )
  config_h_data.set(
    'HAVE_KRB5_AUTHENTICATOR_CLIENT',
    cc.has_member(
      'krb5_authenticator',
      'client',
      prefix: '#include <krb5.h>',
      dependencies: krb5_dep,
    ),
  )
  config_h_data.set(
    'HAVE_KRB5_AUTH_CON_GETAUTHENTICATOR_DOUBLE_POINTER',
    cc.compiles(
      '''
#include <krb5.h>
krb5_error_code krb5_auth_con_getauthenticator(krb5_context, krb5_auth_context, krb5_authenticator**);
      ''',
      dependencies: krb5_dep,
      name: 'krb5_auth_con_getauthenticator has krb5_authenticator** parameter',
    ),
  )
  if not config_h_data.get('HAVE_KRB5_AUTHENTICATOR_CLIENT')
    asn1_lib = cc.find_library(
      'asn1',
      dirs: krb5_dep.get_variable(pkgconfig: 'libdir'),
    )
  endif
endif

configure_file(
  output: 'config.h',
  configuration: config_h_data,
)

add_project_arguments(
  '-DHAVE_CONFIG_H',
  language: 'c',
)

rundir = get_option('rundir')
if rundir == ''
  if false # meson.version().version_compare('>= ')
    runstatedir = get_option('prefix') / get_option('runstatedir')
  else
    runstatedir = get_option('prefix') / get_option('localstatedir') / 'run'
  endif
else
  runstatedir = rundir
endif

install_data(
  sources: 'ksmbd.conf.example',
  install_dir: get_option('sysconfdir') / 'ksmbd',
)

systemdsystemunitdir = get_option('systemdsystemunitdir')
if systemdsystemunitdir == ''
  systemdsystemunitdir = systemd_dep.get_variable(
    pkgconfig: 'systemdsystemunitdir',
    default_value: '',
  )
endif

in_data = configuration_data({
  'sbindir': get_option('prefix') / get_option('sbindir'),
  'sysconfdir': get_option('prefix') / get_option('sysconfdir'),
  'runstatedir': runstatedir,
  'ksmbd_tools_version': meson.project_version(),
})

configure_file(
  input: 'ksmbd.conf.5.in',
  output: 'ksmbd.conf.5',
  install_dir: get_option('mandir') / 'man5',
  configuration: in_data,
)

configure_file(
  input: 'ksmbdpwd.db.5.in',
  output: 'ksmbdpwd.db.5',
  install_dir: get_option('mandir') / 'man5',
  configuration: in_data,
)

configure_file(
  input: 'ksmbd.service.in',
  output: 'ksmbd.service',
  install_dir: systemdsystemunitdir,
  configuration: in_data,
)

subdir('addshare')
subdir('adduser')
subdir('control')
subdir('mountd')
subdir('tools')
