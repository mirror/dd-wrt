#! /bin/sh

if ! pkg-config --version > /dev/null 2>&1 ; then
  echo 'pkg-config not found.' >&2
  echo 'Make sure that pkg-config is installed on your system.' >&2
  exit 1
fi

if autoreconf --version > /dev/null 2>&1 ; then
  exec autoreconf -ivf
fi

if glibtoolize --version > /dev/null 2>&1; then
  LIBTOOLIZE='glibtoolize'
else
  LIBTOOLIZE='libtoolize'
fi

src/libevent-modified/autogen.sh &
cpid1=$!

src/libsodium/autogen.sh &
cpid2=$!

$LIBTOOLIZE --ltdl && \
aclocal && \
autoheader && \
automake --add-missing --force-missing --include-deps && \
autoconf

wait $cpid1
wait $cpid2

