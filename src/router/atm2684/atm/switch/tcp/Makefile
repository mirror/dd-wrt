LIBS=-lsw -latmd -L..
LIBDEPS=../../lib/libatmd.a ../libsw.a
INCLUDES=-I../../qgen
OBJS=tcpsw.o
PGMS=sw_tcp

include ../Rules.make

sw_tcp:		$(OBJS)
		$(CC) $(LDFLAGS) -o sw_tcp $(OBJS) $(LIBS) $(LDLIBS)
