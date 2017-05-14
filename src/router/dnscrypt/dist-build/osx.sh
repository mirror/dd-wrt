#! /bin/sh

export CFLAGS="-mmacosx-version-min=10.10 -march=core2 -O2 -g"
export LDFLAGS="-mmacosx-version-min=10.10 -march=core2 -O2 -g"

./configure --with-included-ltdl \
            --enable-plugins-root && \
make -j3
