LDLIBS=-latmd
INCLUDES=-I../qgen -I../saal
BOOTPGMS=atmaddr esi
SYSPGMS=atmloop atmtcp enitune zntune # nstune
USRPGMS=atmdiag atmdump sonetdiag saaldump
MAN8=atmaddr.8 atmdiag.8 atmdump.8 atmloop.8 atmtcp.8 esi.8

include ../Rules.make

saaldump:	saaldump.o
		$(CC) $(LDFLAGS) -o saaldump saaldump.o ../qgen/qd.dump.o \
		  -L../saal -lsaal -L../lib -latm -latmd
