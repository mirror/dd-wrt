sbin_PROGRAMS = les bus lecs

LDADD = $(top_builddir)/src/lib/libatm.la

LES_BUS_SRCS = mem.c mem.h load.c load.h units.c units.h load_lex.l load_lex.h \
		timers.c timers.h dump.c dump.h lane_atm.c lane_atm.h \
		events.c events.h lane.c lane.h
les_SOURCES = $(LES_BUS_SRCS) packet.c packet.h connect.c connect.h db.c db.h
bus_SOURCES = $(LES_BUS_SRCS) connect_bus.c
lecs_SOURCES = lecs_db.l lecs_db.h lecs_load.c lecs_load.h lecs.c lecs.h \
		ldb.c ldb.h mem_lecs.c mem_lecs.h atm_lecs.c atm_lecs.h

man_MANS = les.8 lecs.8 bus.8

EXTRA_DIST = $(man_MANS) COPYRIGHT.TUT USAGE

