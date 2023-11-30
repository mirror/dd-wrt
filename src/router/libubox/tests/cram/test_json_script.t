set test bin path:

  $ [ -n "$TEST_BIN_DIR" ] && export PATH="$TEST_BIN_DIR:$PATH"
  $ export TEST_INPUTS="$TESTDIR/inputs"
  $ alias js="valgrind --quiet --leak-check=full test-json-script"
  $ alias js-san="test-json-script-san"

check that json-script is producing expected results:

  $ js
  Usage: test-json-script [VARNAME=value] <filename_json_script>
  [254]

  $ js-san
  Usage: test-json-script-san [VARNAME=value] <filename_json_script>
  [254]

  $ echo '}' > test.json; js test.json
  load JSON data from test.json failed.

  $ echo '}' > test.json; js-san test.json
  load JSON data from test.json failed.

  $ js nada.json 2>&1 | grep load.*failed
  load JSON data from nada.json failed.

  $ js-san nada.json 2>&1 | grep load.*failed
  load JSON data from nada.json failed.

  $ echo '[ [ ] [ ] ]' > test.json; js test.json
  load JSON data from test.json failed.

  $ echo '[ [ ] [ ] ]' > test.json; js-san test.json
  load JSON data from test.json failed.

check example json-script:

  $ js $TEST_INPUTS/json-script.json
  exec  /%/
  exec_if_or 

  $ js-san $TEST_INPUTS/json-script.json
  exec  /%/
  exec_if_or 

  $ js EXECVAR=meh ORVAR=meep $TEST_INPUTS/json-script.json
  exec meh /%/
  exec_if_or meep

  $ js-san EXECVAR=meh ORVAR=meep $TEST_INPUTS/json-script.json
  exec meh /%/
  exec_if_or meep

check has expression:

  $ echo '
  > [
  >   [ "if",
  >     [ "has", "VAR" ],
  >     [ "echo", "bar" ],
  >     [ "echo", "baz" ]
  >   ]
  > ]' > test.json

  $ js VAR=foo test.json
  echo bar

  $ js-san VAR=foo test.json
  echo bar

  $ js VAR=bar test.json
  echo bar

  $ js-san VAR=bar test.json
  echo bar

  $ js test.json
  echo baz

  $ js-san test.json
  echo baz

check eq expression:

  $ echo '
  > [
  >   [ "if",
  >     [ "eq", "VAR", "bar" ],
  >     [ "echo", "foo" ],
  >     [ "echo", "baz" ]
  >   ]
  > ]' > test.json

  $ js VAR=bar test.json
  echo foo

  $ js-san VAR=bar test.json
  echo foo

  $ js VAR=xxx test.json
  echo baz

  $ js-san VAR=xxx test.json
  echo baz

  $ js test.json
  echo baz

  $ js-san test.json
  echo baz

check regex single expression:

  $ echo '
  > [
  >   [ "if",
  >     [ "regex", "VAR", ".ell." ],
  >     [ "echo", "bar" ],
  >     [ "echo", "baz" ]
  >   ]
  > ]' > test.json

  $ js VAR=hello test.json
  echo bar

  $ js-san VAR=hello test.json
  echo bar

  $ js VAR=.ell. test.json
  echo bar

  $ js-san VAR=.ell. test.json
  echo bar

  $ js test.json
  echo baz

  $ js-san test.json
  echo baz

  $ js VAR= test.json
  echo baz

  $ js-san VAR= test.json
  echo baz

  $ js VAR=hell test.json
  echo baz

  $ js-san VAR=hell test.json
  echo baz
