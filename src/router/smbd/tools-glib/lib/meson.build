core_files = [
  'management/tree_conn.c',
  'management/user.c',
  'management/share.c',
  'management/session.c',
  'config_parser.c',
  'ksmbdtools.c',
]

if krb5_dep.found()
  core_files += [
    'management/spnego.c',
    'asn1.c',
    'management/spnego_krb5.c',
  ]
endif

libksmbdtools = static_library('ksmbdtools', core_files, include_directories: tools_incdir, dependencies: [glib_dep, krb5_dep, asn1_lib])
