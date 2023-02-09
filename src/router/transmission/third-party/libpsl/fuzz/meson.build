fuzzer_cargs = [
  '-DSRCDIR="@0@"'.format(meson.current_source_dir()),
  '-DTEST_RUN',
]

foreach test_case : ['fuzzer', 'load_fuzzer', 'load_dafsa_fuzzer']
  if enable_runtime == 'libidn2'
    test_name = 'libpsl_idn2_' + test_case
  elif enable_runtime == 'libicu'
    test_name = 'libpsl_icu_' + test_case
  elif enable_runtime == 'libidn'
    test_name = 'libpsl_idn_' + test_case
  else
    test_name = 'libpsl_' + test_case
  endif
  source_file = 'libpsl_@0@.c'.format(test_case)
  exe = executable(test_name, source_file, 'main.c',
    c_args : fuzzer_cargs,
    include_directories : configinc,
    link_language : link_language,
    dependencies : [libpsl_dep, libicu_dep]
  )
  test(test_name, exe)
endforeach
