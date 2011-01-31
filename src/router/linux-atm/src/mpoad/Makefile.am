sbin_PROGRAMS = mpcd

LDADD = $(top_builddir)/src/lib/libatm.la

mpcd_SOURCES = get_vars.c get_vars.h io.c io.h k_interf.c k_interf.h main.c \
		p_factory.c  p_recogn.c id_list.c tag_list.c \
		poll2select.c poll2select.h lecs.c lecs.h packets.h

man_MANS = mpcd.8

EXTRA_DIST = $(man_MANS) CHANGELOG README.mpoa TODO

