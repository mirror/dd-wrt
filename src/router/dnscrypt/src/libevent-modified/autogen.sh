#!/bin/sh
if [ -x "`which autoreconf 2>/dev/null`" ] ; then
   exec autoreconf -ivf
fi

$LIBTOOLIZE && \
aclocal -I m4 && \
autoheader && \
automake --add-missing --force-missing --include-deps && \
autoconf -I m4
