AM_CFLAGS = @PTHREAD_CFLAGS@

noinst_LIBRARIES = libdht.a
libdht_a_SOURCES = dht.c
libdht_a_DEPENDENCIES = $(builddir)/include/dht
noinst_HEADERS = dht.h
EXTRA_DIST = CHANGES CMakeLists.txt dht-example.c LICENCE README

$(builddir)/include/dht:
	$(MKDIR_P) $(builddir)/include
	(cd $(builddir)/include && $(LN_S) $(abs_srcdir) dht)
