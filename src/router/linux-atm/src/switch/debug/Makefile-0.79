LIBS=-lsw -latmd -L..
LIBDEPS=../../lib/libatmd.a ../libsw.a
INCLUDES=-I../../qgen
OBJS=debug.o
PGMS=sw_debug

include ../Rules.make

sw_debug:	$(OBJS)
		$(CC) $(LDFLAGS) -o sw_debug $(OBJS) $(LIBS) $(LDLIBS)
