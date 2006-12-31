LIBS=-L../saal -lsaal -latmd -lfl # lex may want  -ll  here
LIBDEPS=../saal/libsaal.a ../lib/libatmd.a
INCLUDES=-I../qgen -I../saal -I.
OBJS=atmsigd.o io.o kernel.o mess.o proto.o uni.o sap.o timeout.o trace.o \
  policy.o lex.yy.o y.tab.o
EXTOBJS=../qgen/q.out.o ../qgen/qd.dump.o
BOOTPGMS=atmsigd
TRASH=mess.c
TRASH_SPOTLESS=q.out.h
MAN4=atmsigd.conf.4
MAN8=atmsigd.8

include ../Rules.make
CFLAGS_NOWARN += $(STANDARDS)

atmsigd:		$(OBJS) $(EXTOBJS)
			$(CC) $(LDFLAGS) -o atmsigd $(OBJS) $(EXTOBJS) \
			  $(LDLIBS) $(LIBS)

lex.yy.c:		cfg.l y.tab.h ../lib/atm.h
			$(LEX) cfg.l

y.tab.c y.tab.h:	cfg.y ../lib/atmd.h proto.h io.h
			$(YACC) -d cfg.y

mess.c:			../qgen/uni.h mkmess.pl
			perl ./mkmess.pl <../qgen/uni.h >mess.c

#
# The following hack makes sure that "make depend" finds q.out.h and is
# happy with it. Once qgen has been built, there will be ../qgen/q.out.h,
# which is first in the include file search path and therefore gets
# included. An second "make depend" will also use the right file.
#

$(DEPEND):		fake_q.out.h

fake_q.out.h:
			echo "! This must not compile" >q.out.h
