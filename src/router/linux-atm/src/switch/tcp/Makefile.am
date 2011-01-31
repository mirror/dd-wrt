noinst_PROGRAMS = sw_tcp

INCLUDES = -I$(srcdir)/../../q2931

sw_tcp_SOURCES = tcpsw.c
sw_tcp_XTRAS = $(top_builddir)/src/switch/libsw.a \
		$(top_builddir)/src/lib/libatm.la
sw_tcp_LDADD = $(sw_tcp_XTRAS) 
sw_tcp_DEPENDENCIES = $(sw_tcp_XTRAS)

EXTRA_DIST = mkfiles README

