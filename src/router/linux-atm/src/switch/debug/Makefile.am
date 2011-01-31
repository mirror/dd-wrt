noinst_PROGRAMS = sw_debug

INCLUDES = -I$(srcdir)/../../q2931

sw_debug_SOURCES = debug.c
sw_debug_XTRAS = $(top_builddir)/src/switch/libsw.a \
			$(top_builddir)/src/lib/libatm.la
sw_debug_LDADD = $(sw_debug_XTRAS) 
			
sw_debug_DEPENDENCIES = $(sw_debug_XTRAS)

EXTRA_DIST = demo README

