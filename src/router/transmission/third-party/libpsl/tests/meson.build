psl_dafsa = custom_target('psl.dafsa',
  input : psl_file,
  output : 'psl.dafsa',
  build_by_default: false,
  command : [python, psl_make_dafsa, '--output-format=binary', '@INPUT@', '@OUTPUT@'])

psl_ascii_dafsa = custom_target('psl_ascii.dafsa',
  input : psl_file,
  output : 'psl_ascii.dafsa',
  build_by_default: false,
  command : [python, psl_make_dafsa, '--output-format=binary', '--encoding=ascii', '@INPUT@', '@OUTPUT@'])

fsmod = import('fs')
tests_cargs = [
  '-DHAVE_CONFIG_H',
  '-DSRCDIR="@0@"'.format(meson.current_source_dir()),
  '-DPSL_FILE="@0@"'.format(psl_file),
  '-DPSL_TESTFILE="@0@"'.format(psl_test_file),
  '-DPSL_DAFSA="@0@"'.format(fsmod.as_posix(psl_dafsa.full_path())),
  '-DPSL_ASCII_DAFSA="@0@"'.format(fsmod.as_posix(psl_ascii_dafsa.full_path())),
]

tests = [
  'test-is-public',
  'test-is-public-all',
  'test-is-cookie-domain-acceptable',
]

if enable_builtin
  tests += ['test-is-public-builtin', 'test-registrable-domain']
endif

libtestcommon = static_library('testcommon', 'common.c',
  build_by_default: false)

foreach test_name : tests
  sources = [test_name + '.c', 'common.c', 'common.h']
  exe = executable(test_name, sources,
    build_by_default: false,
    c_args : tests_cargs,
    link_with : [libpsl, libtestcommon],
    include_directories : configinc,
    link_language : link_language,
    dependencies : [libpsl_dep, networking_deps])
  test(test_name, exe, depends : [psl_dafsa, psl_ascii_dafsa])
endforeach
