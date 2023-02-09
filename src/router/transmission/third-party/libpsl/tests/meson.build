psl_dafsa = custom_target('psl.dafsa',
  input : psl_file,
  output : 'psl.dafsa',
  command : [python, psl_make_dafsa, '--output-format=binary', '@INPUT@', '@OUTPUT@'])

psl_ascii_dafsa = custom_target('psl_ascii.dafsa',
  input : psl_file,
  output : 'psl_ascii.dafsa',
  command : [python, psl_make_dafsa, '--output-format=binary', '--encoding=ascii', '@INPUT@', '@OUTPUT@'])

tests_cargs = [
  '-DHAVE_CONFIG_H',
  '-DSRCDIR="@0@"'.format(meson.current_source_dir()),
  '-DPSL_FILE="@0@"'.format(psl_file),
  '-DPSL_TESTFILE="@0@"'.format(psl_test_file),
  '-DPSL_DAFSA="@0@"'.format(psl_dafsa.full_path()),
  '-DPSL_ASCII_DAFSA="@0@"'.format(psl_ascii_dafsa.full_path()),
]

tests = [
  'test-is-public',
  'test-is-public-all',
  'test-is-cookie-domain-acceptable',
]

if enable_builtin != 'no'
  tests += ['test-is-public-builtin', 'test-registrable-domain']
endif

foreach test_name : tests
  source = test_name + '.c'
  exe = executable(test_name, source,
    c_args : tests_cargs,
    link_with : libpsl,
    include_directories : configinc,
    link_language : link_language,
    dependencies : [libpsl_dep, networking_deps])
  test(test_name, exe, depends : [psl_dafsa, psl_ascii_dafsa])
endforeach
