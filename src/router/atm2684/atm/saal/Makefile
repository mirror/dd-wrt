LIBS=
OBJS=pdu.o saal.o sscf.o sscop.o queue.o

do_all:				libsaal.a

include ../Rules.make

../saal/libsaal.a:		libsaal.a

libsaal.a:			$(OBJS)
				ar rcs libsaal.a $(OBJS)
