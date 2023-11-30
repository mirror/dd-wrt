check that blobmsg parsing/checking would produce expected results in procd:

  $ [ -n "$TEST_BIN_DIR" ] && export PATH="$TEST_BIN_DIR:$PATH"
  $ export INPUTS="$TESTDIR/inputs"

  $ for blob in $(LC_ALL=C find $INPUTS -type f | sort ); do
  >   valgrind --quiet --leak-check=full test-blobmsg-procd-instance $blob; \
  >   test-blobmsg-procd-instance-san $blob; \
  > done
  procd-instance-nlbwmon.bin: OK
  procd-instance-nlbwmon.bin: OK
