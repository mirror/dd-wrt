psl_make_dafsa = files('psl-make-dafsa')

suffixes_dafsa_h = custom_target('suffixes_dafsa.h',
  input : psl_file,
  output : 'suffixes_dafsa.h',
  command : [python, psl_make_dafsa, '--output-format=cxx+', '@INPUT@', '@OUTPUT@'])

sources = [
  'lookup_string_in_fixed_set.c',
  'psl.c',
]

cargs = [
  '-DHAVE_CONFIG_H',
  '-DBUILDING_PSL',
  '-DPSL_DISTFILE="@0@"'.format(psl_distfile),
]

fs = import('fs')
libtool_version_info = fs.read(meson.project_source_root() / 'libtool_version_info.txt').strip().split(':')
libtool_version_current = libtool_version_info[0].to_int()
libtool_version_revision = libtool_version_info[1].to_int()
libtool_version_age = libtool_version_info[2].to_int()
library_version = '@0@.@1@.@2@'.format(libtool_version_current - libtool_version_age, libtool_version_age, libtool_version_revision)

interface_cargs = []
if host_machine.system() == 'windows'
    default_library = get_option('default_library')
    if default_library == 'static'
        interface_cargs = ['-DPSL_STATIC']
        cargs += interface_cargs
    elif default_library == 'both'
        warning('Building both static and dynamic libraries. -DPSL_STATIC will not be added to pkg-config metadata')
    endif
endif

libpsl = library('psl', sources, suffixes_dafsa_h,
  include_directories : [configinc, includedir],
  c_args : cargs,
  dependencies : [libidn2_dep, libidn_dep, libicu_dep, libunistring, networking_deps, libiconv_dep],
  version: library_version,
  install: true,
  link_language : link_language
)

pkgconfig.generate(libpsl,
  name : 'libpsl',
  description : 'Public Suffix List C library',
  extra_cflags : interface_cargs)

libpsl_dep = declare_dependency(link_with : libpsl,
  include_directories : includedir,
  compile_args : interface_cargs)

install_data('psl-make-dafsa', install_dir : get_option('bindir'))
install_man('psl-make-dafsa.1')
