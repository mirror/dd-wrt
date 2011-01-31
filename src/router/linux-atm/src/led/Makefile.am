sbin_PROGRAMS = zeppelin

zeppelin_SOURCES = join.c join.h conn.c conn.h main.c address.c address.h \
			frames.c frames.h display.c display.h \
			kernel.c kernel.h frame_defs.h lec.h
zeppelin_LDADD = $(top_builddir)/src/lib/libatm.la
zeppelin_DEPENDENCIES = $(zeppelin_LDADD)

man_MANS = zeppelin.8

EXTRA_DIST = $(man_MANS) COPYRIGHT.TUT

