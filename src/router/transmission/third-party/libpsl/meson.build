project('libpsl', 'c',
  version : files('version.txt'),
  meson_version : '>=0.60.0')

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
if ['libidn2', 'auto'].contains(enable_runtime)
  libidn2_dep = dependency('libidn2', required : false)
  if not libidn2_dep.found() and cc.has_header('idn2.h')
    libidn2_dep = cc.find_library('idn2', required : false)
  endif
  if libidn2_dep.found()
    if enable_runtime == 'auto'
      enable_runtime = 'libidn2'
    endif
  elif enable_runtime == 'libidn2'
    error('You requested libidn2 but it is not installed.')
  endif
endif

if ['libicu', 'auto'].contains(enable_runtime)
  libicu_dep = dependency('icu-uc', 'ICU',
                          components: 'uc',
                          required : false)
  if libicu_dep.found()
    if enable_runtime == 'auto'
      enable_runtime = 'libicu'
    endif
    if add_languages('cpp', native : false)
        link_language = 'cpp'
    else
        error('C++ compiler is not available')
    endif
  elif enable_runtime == 'libicu'
    error('You requested libicu but it is not installed.')
  endif
endif

if ['libidn', 'auto'].contains(enable_runtime)
  libidn_dep = dependency('libidn', required : false)
  if not libidn_dep.found() and cc.has_header('idna.h')
    libidn_dep = cc.find_library('idn', required : false)
  endif
  if libidn_dep.found()
    if enable_runtime == 'auto'
      enable_runtime = 'libidn'
    endif
  elif enable_runtime == 'libidn'
    error('You requested libidn but it is not installed.')
  endif
endif

if libidn2_dep.found() or libidn_dep.found()
  # Check for libunistring, we need it for psl_str_to_utf8lower()
  libunistring = cc.find_library('unistring')
  libiconv_dep = dependency('iconv')
endif

if host_machine.system() == 'windows'
  networking_deps = cc.find_library('ws2_32')
endif

if enable_runtime == 'auto'
  enable_runtime = 'no'
endif

config = configuration_data()
config.set_quoted('PACKAGE_VERSION', meson.project_version())
config.set('WITH_LIBIDN2', enable_runtime == 'libidn2')
config.set('WITH_LIBICU', enable_runtime == 'libicu')
config.set('WITH_LIBIDN', enable_runtime == 'libidn')
config.set('ENABLE_BUILTIN', enable_builtin)
config.set('HAVE_UNISTD_H', cc.check_header('unistd.h'))
config.set('HAVE_STDINT_H', cc.check_header('stdint.h'))
config.set('HAVE_DIRENT_H', cc.check_header('dirent.h'))
config.set('HAVE_CLOCK_GETTIME', cc.has_function('clock_gettime'))
config.set('HAVE_FMEMOPEN', cc.has_function('fmemopen'))
config.set('HAVE_NL_LANGINFO', cc.has_function('nl_langinfo'))
if cc.has_function_attribute('visibility')
  config.set('HAVE_VISIBILITY', 1)
endif
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

python = find_program('python3')
pkgconfig = import('pkgconfig')

if cc.get_id() == 'msvc'
  if not cc.has_header_symbol('stdio.h', 'snprintf')
    if cc.has_header_symbol('stdio.h', '_snprintf')
      add_project_arguments('-Dsnprintf=_snprintf', language: 'c')
    endif
  endif
endif

subdir('include')
subdir('src')
subdir('tools')
if get_option('tests')
  subdir('tests')
  subdir('fuzz')
endif
subdir(join_paths('docs', 'libpsl'))
