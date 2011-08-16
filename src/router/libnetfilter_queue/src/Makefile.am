# This is _NOT_ the library release version, it's an API version.
# Please read Chapter 6 "Library interface versions" of the libtool documentation before making any modification
LIBVERSION=2:0:1

INCLUDES = $(all_includes) -I$(top_srcdir)/include -I${KERNELDIR}
AM_CFLAGS = -fPIC -Wall
LIBS = @LIBNFQUEUE_LIBS@

lib_LTLIBRARIES = libnetfilter_queue.la

libnetfilter_queue_la_LDFLAGS = -Wc,-nostartfiles -lnfnetlink \
				-version-info $(LIBVERSION)
libnetfilter_queue_la_SOURCES = libnetfilter_queue.c 
