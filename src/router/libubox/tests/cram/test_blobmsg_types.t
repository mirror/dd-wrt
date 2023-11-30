check that blobmsg is producing expected results:

  $ [ -n "$TEST_BIN_DIR" ] && export PATH="$TEST_BIN_DIR:$PATH"

  $ valgrind --quiet --leak-check=full test-blobmsg-types
  [*] blobmsg dump:
  string: Hello, world!
  int64_max: 9223372036854775807
  int64_min: -9223372036854775808
  int32_max: 2147483647
  int32_min: -2147483648
  int16_max: 32767
  int16_min: -32768
  int8_max: 127
  int8_min: -128
  double_max: 1.797693e+308
  double_min: 2.225074e-308
  [*] blobmsg dump cast_u64:
  string: Hello, world!
  int64_max: 9223372036854775807
  int64_min: 9223372036854775808
  int32_max: 2147483647
  int32_min: 2147483648
  int16_max: 32767
  int16_min: 32768
  int8_max: 127
  int8_min: 128
  double_max: 1.797693e+308
  double_min: 2.225074e-308
  [*] blobmsg dump cast_s64:
  string: Hello, world!
  int64_max: 9223372036854775807
  int64_min: -9223372036854775808
  int32_max: 2147483647
  int32_min: -2147483648
  int16_max: 32767
  int16_min: -32768
  int8_max: 127
  int8_min: -128
  double_max: 1.797693e+308
  double_min: 2.225074e-308
  
  [*] blobmsg to json: {"string":"Hello, world!","int64_max":9223372036854775807,"int64_min":-9223372036854775808,"int32_max":2147483647,"int32_min":-2147483648,"int16_max":32767,"int16_min":-32768,"int8_max":true,"int8_min":true,"double_max":179769313486231570814527423731704356798070567525844996598917476803157260780028538760589558632766878171540458953514382464234321326889464182768467546703537516986049910576551282076245490090389328944075868508455133942304583236903222948165808559332123348274797826204144723168738177180919299881250404026184124858368.000000,"double_min":0.000000}
  
  [*] blobmsg from json:
  string: Hello, world!
  int64_max: 9223372036854775807
  int64_min: -9223372036854775808
  int32_max: 2147483647
  int32_min: -2147483648
  int16_max: 32767
  int16_min: -32768
  int8_max: 1
  int8_min: 1
  double_max: 1.797693e+308
  double_min: 0.000000e+00
  
  [*] blobmsg from json/cast_u64:
  string: Hello, world!
  int64_max: 9223372036854775807
  int64_min: 9223372036854775808
  int32_max: 2147483647
  int32_min: 2147483648
  int16_max: 32767
  int16_min: 4294934528
  int8_max: 1
  int8_min: 1
  double_max: 1.797693e+308
  double_min: 0.000000e+00
  
  [*] blobmsg from json/cast_s64:
  string: Hello, world!
  int64_max: 9223372036854775807
  int64_min: -9223372036854775808
  int32_max: 2147483647
  int32_min: -2147483648
  int16_max: 32767
  int16_min: -32768
  int8_max: 1
  int8_min: 1
  double_max: 1.797693e+308
  double_min: 0.000000e+00
