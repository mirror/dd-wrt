#!/bin/sh

set -e

if [ ! -e wolfssl ]; then
    git clone --depth=1 https://github.com/wolfSSL/wolfssl.git
fi
cd wolfssl
git clean -f
./autogen.sh
options="
    --enable-all
    --enable-openssh
    --enable-opensslextra
    --prefix=$PWD/../wolfssl-inst
"
./configure ${options}
make -j"$(nproc)"
make install
