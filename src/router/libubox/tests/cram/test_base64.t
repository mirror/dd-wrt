set test bin path:

  $ [ -n "$TEST_BIN_DIR" ] && export PATH="$TEST_BIN_DIR:$PATH"

check that base64 is producing expected results:

  $ valgrind --quiet --leak-check=full test-b64
  0 
  4 Zg==
  4 Zm8=
  4 Zm9v
  8 Zm9vYg==
  8 Zm9vYmE=
  8 Zm9vYmFy
  0 
  1 f
  2 fo
  3 foo
  4 foob
  5 fooba
  6 foobar

  $ test-b64-san
  0 
  4 Zg==
  4 Zm8=
  4 Zm9v
  8 Zm9vYg==
  8 Zm9vYmE=
  8 Zm9vYmFy
  0 
  1 f
  2 fo
  3 foo
  4 foob
  5 fooba
  6 foobar

check that b64_encode and b64_decode assert invalid input

  $ alias check="grep Assertion output.log | sed 's;.*\(b64_.*code\).*\(Assertion.*$\);\1: \2;' | LC_ALL=C sort"

  $ test-b64_decode > output.log 2>&1; check
  b64_decode: Assertion `dest && targsize > 0' failed.

  $ test-b64_encode > output.log 2>&1; check
  b64_encode: Assertion `dest && targsize > 0' failed.

  $ test-b64_decode-san > output.log 2>&1; check
  b64_decode: Assertion `dest && targsize > 0' failed.

  $ test-b64_encode-san > output.log 2>&1; check
  b64_encode: Assertion `dest && targsize > 0' failed.
