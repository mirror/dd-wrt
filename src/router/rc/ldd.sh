#!/bin/sh
if [ -L /lib/ld-musl* ]
then
	#MUSL
        ln -s /lib/ld-musl* /tmp/ldd >/dev/null 2>&1
        /tmp/ldd $1
else
	#uclibc
        LD_TRACE_LOADED_OBJECTS=1 $1
fi
