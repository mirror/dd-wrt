#!/bin/sh -eu
#
# Copyright(c) 2017-2018 Tim Ruehsen
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.
#
# This file is part of libpsl.

srcdir="${srcdir:-.}"
export LD_LIBRARY_PATH=${srcdir}/../lib/.libs/

cat ${srcdir}/../config.log|grep afl-clang-fast >/dev/null 2>&1
if test $? != 0; then
	echo "compile first library as:"
	echo "CC=afl-clang-fast ./configure"
	exit 1
fi

if test -z "$1"; then
	echo "Usage: $0 test-case"
	echo "Example: $0 libpsl_fuzzer"
	exit 1
fi

rm -f $1
CFLAGS="-g -O2" CC=afl-clang-fast make "$1" || exit 1

### minimize test corpora
if test -d ${fuzzer}.in; then
  mkdir -p ${fuzzer}.min
  for i in `ls ${fuzzer}.in`; do
    fin="${fuzzer}.in/$i"
    fmin="${fuzzer}.min/$i"
    if ! test -e $fmin || test $fin -nt $fmin; then
      afl-tmin -i $fin -o $fmin -- ./${fuzzer}
    fi
  done
fi

TMPOUT=${fuzzer}.$$.out
mkdir -p ${TMPOUT}

if test -f ${fuzzer}.dict; then
  afl-fuzz -i ${fuzzer}.min -o ${TMPOUT} -x ${fuzzer}.dict -- ./${fuzzer}
else
  afl-fuzz -i ${fuzzer}.min -o ${TMPOUT} -- ./${fuzzer}
fi

echo "output was stored in $TMPOUT"

exit 0
