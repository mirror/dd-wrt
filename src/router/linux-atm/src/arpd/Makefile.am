LDADD = $(top_builddir)/src/lib/libatm.la

sbin_PROGRAMS = atmarp atmarpd

atmarp_SOURCES = atmarp.c atmarp.h atmarpd.h
atmarpd_SOURCES = atmarpd.c atmarpd.h arp.c arp.h io.c io.h itf.c itf.h \
			table.c table.h atmarp.h

include_HEADERS = atmarp.h atmarpd.h

man_MANS = atmarp.8 atmarpd.8

EXTRA_DIST = $(man_MANS)

