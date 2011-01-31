SUBDIRS = . debug tcp

noinst_PROGRAMS = swc
noinst_LIBRARIES = libsw.a

swc_SOURCES = swc.c swc.h
swc_LDADD = $(top_builddir)/src/lib/libatm.la
swc_DEPENDENCIES = $(swc_LDADD) $(top_builddir)/src/include/atm.h \
			$(top_builddir)/src/include/atmd.h
libsw_a_SOURCES = control.c dispatch.c dispatch.h proto.c proto.h relay.c \
			route.c route.h sig.c sig.h cfg_y.y cfg_l.l fab.h

EXTRA_DIST = cfg_y.h README

