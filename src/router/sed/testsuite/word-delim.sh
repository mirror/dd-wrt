#!/bin/sh
# Exercise the DFA regression in sed-4.6.
. "${srcdir=.}/testsuite/init.sh"; path_prepend_ ./sed
print_ver_ sed

require_en_utf8_locale_

# Also ensure that this works in both the C locale and that multibyte one.
# In the C locale, it failed due to a dfa.c regression in sed-4.6.
echo 123-x > in || framework_failure_
echo 123 > exp || framework_failure_

for locale in C en_US.UTF-8; do
  LC_ALL=$locale sed 's/.\bx//' in > out 2>err || fail=1
  compare exp out || fail=1
  compare /dev/null err || fail=1
done

Exit $fail
