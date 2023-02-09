#!/bin/bash -eu

# 1. execute 'make fuzz-coverage' in the top directory
# 2. execute './view-coverage.sh <fuzz target>
#    example: ./view-coverage.sh libpsl_fuzzer

if test -z "$1"; then
  echo "Usage: $0 <fuzz target>"
  echo "Example: $0 libpsl_fuzzer"
  exit 1
fi

fuzzer="./"$1
LCOV_INFO=coverage.info
#make fuzz-coverage CFLAGS="$(CFLAGS) --coverage" LDFLAGS="$(LDFLAGS) --coverage"
./coverage.sh $fuzzer
lcov --capture --initial --directory ../src --directory . --output-file $LCOV_INFO
lcov --capture --directory ../src --output-file $LCOV_INFO
#lcov --remove $LCOV_INFO '*/test_linking.c' '*/css_tokenizer.lex' '*/<stdout>' '*/*.h' -o $LCOV_INFO
genhtml --prefix . --ignore-errors source $LCOV_INFO --legend --title "$1" --output-directory=lcov
xdg-open lcov/index.html
