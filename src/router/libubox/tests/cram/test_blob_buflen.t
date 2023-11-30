check that blob buffer cannot exceed maximum buffer length:

  $ [ -n "$TEST_BIN_DIR" ] && export PATH="$TEST_BIN_DIR:$PATH"

  $ valgrind --quiet --leak-check=full test-blob-buflen
  SUCCESS: failed to allocate attribute

  $ test-blob-buflen-san
  SUCCESS: failed to allocate attribute
