noinst_LIBRARIES = libminiupnp.a

AM_CFLAGS = @PTHREAD_CFLAGS@ -DNDEBUG -D_GNU_SOURCE

libminiupnp_a_SOURCES = \
	connecthostport.c \
	igd_desc_parse.c \
	minisoap.c \
	minissdpc.c \
	miniupnpc.c \
	miniwget.c \
	minixml.c \
	portlistingparse.c \
	receivedata.c \
	upnpcommands.c \
	upnpdev.c \
	upnperrors.c \
	upnpreplyparse.c

libminiupnp_a_DEPENDENCIES = $(builddir)/include/miniupnp

noinst_HEADERS = \
	codelength.h \
	connecthostport.h \
	igd_desc_parse.h \
	minisoap.h \
	minissdpc.h \
	miniupnpc_declspec.h \
	miniupnpc.h \
	miniupnpctypes.h \
	miniwget.h \
	minixml.h \
	portlistingparse.h \
	receivedata.h \
	upnpcommands.h \
	upnpdev.h \
	upnperrors.h \
	upnpreplyparse.h

EXTRA_DIST = \
	CMakeLists.txt \
	README \
	LICENSE \
	VERSION \
	miniupnpcstrings.h.cmake \
	miniupnpcstrings.h.in \
	updateminiupnpcstrings.sh

BUILT_SOURCES = \
	miniupnpcstrings.h

miniupnpcstrings.h: Makefile
	$(srcdir)/updateminiupnpcstrings.sh $(srcdir)/VERSION $(srcdir)/miniupnpcstrings.h.in $@

$(builddir)/include/miniupnp:
	$(MKDIR_P) $(builddir)/include
	(cd $(builddir)/include && $(LN_S) $(abs_srcdir) miniupnp)

DISTCLEANFILES = $(builddir)/miniupnpcstrings.h
