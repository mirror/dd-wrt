AM_CFLAGS = @PTHREAD_CFLAGS@ 

lib_LTLIBRARIES = libdht.la
libdht_la_SOURCES = dht.c
libdht_la_DEPENDENCIES = $(builddir)/include/dht
noinst_HEADERS = dht.h
EXTRA_DIST = CHANGES CMakeLists.txt dht-example.c LICENCE README

$(builddir)/include/dht:
	$(MKDIR_P) $(builddir)/include
	(cd $(builddir)/include && $(LN_S) $(abs_srcdir) dht)
