check that blobmsg_check_array() is producing expected results:

  $ [ -n "$TEST_BIN_DIR" ] && export PATH="$TEST_BIN_DIR:$PATH"

  $ test-blobmsg_check_array
  Process array_a: entry 0
  array_b contains string: 1
  blobmsg_check_array() test passed
