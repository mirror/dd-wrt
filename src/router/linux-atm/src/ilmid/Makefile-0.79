LIBS = -L../lib -latmd -latm -Lasn1 -lasn1
LIBDEPS=../lib/libatmd.a ../lib/libatm.a
INCLUDES = -I../lib -Iasn1
SRCS = rfc1157_snmp.c rfc1155_smi.c util.c io.c message.c \
       atmf_uni.c mib.c sysgroup.c ilmid.c
OBJS = $(SRCS:%.c=%.o)
BOOTPGMS=ilmid

CLEAN = clean_ilmid
include ../Rules.make
CFLAGS = $(CFLAGS_NOWARN) $(CFLAGS_OPT) $(CFLAGS_PRIVATE) $(STANDARDS)

default: all
 
do_all: ilmid
 
asn1/libasn1.a: 
	@cd asn1; make
 
ilmid: asn1/libasn1.a $(OBJS)
	$(CC) $(OBJS) $(LIBS) -o ilmid

clean_ilmid:
	@cd asn1; make clean
	rm -rf ilmid $(OBJS) *~
