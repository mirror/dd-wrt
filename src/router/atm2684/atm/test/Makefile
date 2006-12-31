ISP_OBJS=isp.o lex.yy.o y.tab.o
USRPGMS=aread awrite ttcp_atm
PGMS=align aping br bw isp window
MAN1=
TRASH=errnos.inc

include ../Rules.make

ttcp_atm.o:		ttcp.c ttcp.o
			gcc -c -w -O2 -o ttcp_atm.o ttcp.c -I../lib

ttcp.o:			# dummy
			touch ttcp.o

isp:			$(ISP_OBJS) ../lib/libatm.a ../lib/libatmd.a
			$(CC) $(LDFLAGS) -o isp $(ISP_OBJS) $(LDLIBS) -latmd \
			  -lfl

lex.yy.c:		ispl.l y.tab.h
			$(LEX) ispl.l

y.tab.c y.tab.h:	ispl.y isp.h ../lib/atm.h
			$(YACC) -d ispl.y

isp.o:			errnos.inc

errnos.inc:		mkerrnos.pl
			perl ./mkerrnos.pl </usr/include/asm/errno.h \
			  >errnos.inc || { rm -f errnos.inc; exit 1; }

#
# During "make depend", we need to have something that keeps the #include from
# failing. The "touch" at the end makes sure that errnos.inc gets rebuilt in
# time. The sleep 1 makes sure that mkerrnos.pl is really more recent than
# errnos.inc
#

$(DEPEND):		fake_errnos.inc

fake_errnos.inc:
			echo "! This must not compile" >errnos.inc
			sleep 1
			touch mkerrnos.pl
