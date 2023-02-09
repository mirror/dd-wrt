project('libpsl', 'c',
  version : files('version.txt'),
  meson_version : '>=0.57.0')

cc = meson.get_compiler('c')

enable_runtime = get_option('runtime')
enable_builtin = get_option('builtin')

# We need to know the build type to determine what .lib files we need on Visual Studio
# for dependencies that don't normally come with pkg-config files for Visual Studio builds
buildtype = get_option('buildtype')

notfound = dependency('', required : false)
libidn2_dep = notfound
libicu_dep = notfound
libidn_dep = notfound
libunistring = notfound
networking_deps = notfound
libiconv_dep = notfound

link_language = 'c'

# FIXME: Cleanup this when Meson gets 'feature-combo':
# https://github.com/mesonbuild/meson/issues/4566
# Dependency fallbacks would help too:
# https://github.com/mesonbuild/meson/pull/4595
if ['libidn2', 'auto'].contains(enable_runtime) or ['libidn2', 'auto'].contains(enable_builtin)
  libidn2_dep = dependency('libidn2', required : false)
  if not libidn2_dep.found() and cc.has_header('idn2.h')
    libidn2_dep = cc.find_library('idn2', required : false)
  endif
  if libidn2_dep.found()
    if enable_runtime == 'auto'
      enable_runtime = 'libidn2'
    endif
    if enable_builtin == 'auto'
      enable_builtin = 'libidn2'
    endif
  elif [enable_runtime, enable_builtin].contains('libidn2')
    error('You requested libidn2 but it is not installed.')
  endif
endif

if ['libicu', 'auto'].contains(enable_runtime) or ['libicu', 'auto'].contains(enable_builtin)
  libicu_dep = dependency('icu-uc', required : false)
  if not libicu_dep.found() and cc.has_header('unicode/ustring.h')
    # MSVC: the debug configuration of ICU generated the libraries with d suffix
    # we must handle this and search for the right library depending on the
    # build type. Note debugoptimized is just a release build with .pdb files enabled
    if cc.get_id() == 'msvc' and buildtype == 'debug'
      libicu_dep = cc.find_library('icuucd', required : false)
    else
      libicu_dep = cc.find_library('icuuc', required : false)
    endif
  endif
  if libicu_dep.found()
    if enable_runtime == 'auto'
      enable_runtime = 'libicu'
    endif
    if enable_builtin == 'auto'
      enable_builtin = 'libicu'
    endif
    if add_languages('cpp', native : false)
        link_language = 'cpp'
    else
        error('C++ compiler is not available')
    endif
  elif [enable_runtime, enable_builtin].contains('libicu')
    error('You requested libicu but it is not installed.')
  endif
endif

if ['libidn', 'auto'].contains(enable_runtime) or ['libidn', 'auto'].contains(enable_builtin)
  libidn_dep = dependency('libidn', required : false)
  if not libidn_dep.found() and cc.has_header('idna.h')
    libidn_dep = cc.find_library('idn', required : false)
  endif
  if libidn_dep.found()
    if enable_runtime == 'auto'
      enable_runtime = 'libidn'
    endif
    if enable_builtin == 'auto'
      enable_builtin = 'libidn'
    endif
  elif [enable_runtime, enable_builtin].contains('libidn')
    error('You requested libidn but it is not installed.')
  endif
endif

if libidn2_dep.found() or libidn_dep.found()
  # Check for libunistring, we need it for psl_str_to_utf8lower()
  libunistring = cc.find_library('unistring')
  found_iconv = false
  if cc.has_function('iconv_open')
    libiconv_dep = []
    found_iconv = true
  endif
  if not found_iconv and cc.has_header_symbol('iconv.h', 'iconv_open')
    libiconv_dep = [cc.find_library('iconv')]
    found_iconv = true
  endif

  if not found_iconv
    error('iconv implementation not found')
  endif
endif

if host_machine.system() == 'windows'
  networking_deps = cc.find_library('ws2_32')
endif

if enable_runtime == 'auto'
  enable_runtime = 'no'
endif
if enable_builtin == 'auto'
  enable_builtin = 'no'
endif

config = configuration_data()
config.set_quoted('PACKAGE_VERSION', meson.project_version())
config.set('WITH_LIBIDN2', enable_runtime == 'libidn2')
config.set('WITH_LIBICU', enable_runtime == 'libicu')
config.set('WITH_LIBIDN', enable_runtime == 'libidn')
config.set('BUILTIN_GENERATOR_LIBIDN2', enable_builtin == 'libidn2')
config.set('BUILTIN_GENERATOR_LIBICU', enable_builtin == 'libicu')
config.set('BUILTIN_GENERATOR_LIBIDN', enable_builtin == 'libidn')
config.set('HAVE_UNISTD_H', cc.check_header('unistd.h'))
config.set('HAVE_STDINT_H', cc.check_header('stdint.h'))
config.set('HAVE_ALLOCA_H', cc.check_header('alloca.h'))
config.set('HAVE_DIRENT_H', cc.check_header('dirent.h'))
config.set('HAVE_ALLOCA', cc.has_function('alloca'))
config.set('HAVE_STRNDUP', cc.has_function('strndup'))
config.set('HAVE_CLOCK_GETTIME', cc.has_function('clock_gettime'))
config.set('HAVE_FMEMOPEN', cc.has_function('fmemopen'))
config.set('HAVE_NL_LANGINFO', cc.has_function('nl_langinfo'))
configure_file(output : 'config.h', configuration : config)

configinc = include_directories('.')
includedir = include_directories('include')

psl_distfile = get_option('psl_distfile')
psl_file = get_option('psl_file')
if psl_file == ''
  psl_file = join_paths(meson.current_source_dir(), 'list', 'public_suffix_list.dat')
endif
psl_test_file = get_option('psl_testfile')
if psl_test_file == ''
  psl_test_file = join_paths(meson.current_source_dir(), 'list', 'tests', 'tests.txt')
endif

python = import('python').find_installation()
pkgconfig = import('pkgconfig')

if cc.get_id() == 'msvc'
  if not cc.has_header_symbol('stdio.h', 'snprintf')
    if cc.has_header_symbol('stdio.h', '_snprintf')
      add_project_arguments('-Dsnprintf=_snprintf', language: 'c')
    endif
  endif
  if cc.has_header_symbol('malloc.h', '_alloca')
    add_project_arguments('-Dalloca=_alloca', language: 'c')
  endif
endif

subdir('include')
subdir('src')
subdir('tools')
subdir('tests')
subdir('fuzz')
subdir(join_paths('docs', 'libpsl'))
