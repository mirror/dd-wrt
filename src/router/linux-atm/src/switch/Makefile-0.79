LDLIBS=-latmd
LIBDEPS=../lib/libatmd.a
SW_OBJS=control.o dispatch.o proto.o relay.o route.o sig.o lex.yy.o y.tab.o
SUBDIRS=debug tcp
PGMS=swc

all:

do_all:			libsw.a

include ../Rules.make

lex.yy.c:		cfg.l y.tab.h ../lib/atm.h
			$(LEX) cfg.l

y.tab.c y.tab.h:	cfg.y ../lib/atmd.h route.h sig.h
			$(YACC) -d cfg.y

libsw.a:		$(SW_OBJS)
			ar rcs libsw.a $(SW_OBJS)
