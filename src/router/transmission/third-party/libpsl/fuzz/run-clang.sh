#!/bin/bash -e
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

trap ctrl_c INT

ctrl_c() {
  ./${fuzzer} -merge=1 ${fuzzer}.in ${fuzzer}.new
  rm -rf ${fuzzer}.new
}

if test -z "$1"; then
	echo "Usage: $0 <fuzzer target>"
	echo "Example: $0 libpsl_idn2_fuzzer"
	exit 1
fi

if ! grep -q FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION Makefile; then
  echo "The fuzzers haven't been built for fuzzing (maybe for regression testing !?)"
  echo "Please built regarding README.md and try again."
  exit 1
fi

# you'll need ~2GB free memory per worker !
fuzzer=$1
workers=$(($(nproc) - 0))
jobs=$workers

if test -n "$BUILD_ONLY"; then
  exit 0
fi

# create directory for NEW test corpora (covering new areas of code)
mkdir -p ${fuzzer}.new

if test -f ${fuzzer}.dict; then
  ./${fuzzer} -dict=${fuzzer}.dict ${fuzzer}.new ${fuzzer}.in -jobs=$jobs -workers=$workers
else
  ./${fuzzer} ${fuzzer}.new ${fuzzer}.in -jobs=$jobs -workers=$workers
fi

exit 0
