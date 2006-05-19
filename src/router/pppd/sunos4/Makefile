#
# Makefile for STREAMS modules for SunOS 4.
#
# $Id: Makefile,v 1.1 2003/07/10 07:43:05 honor Exp $
#

include Makedefs

LD = /usr/bin/ld		# make sure we don't get gnu ld

# Defining __$(ARCH)__ is for gcc's broken version of sun/vddrv.h.
ARCH = `/bin/arch -k`
DEFINES= -DKERNEL -D_KERNEL -DSUNOS4 -D$(ARCH) -D__$(ARCH)__ \
	 -DDEBUG -DNO_DLPI -DSNIT_SUPPORT
CFLAGS= $(DEFINES) -I../include $(COPTS)

MODULES= ppp_mod.o ppp_ahdl_mod.o ppp_comp_mod.o if_ppp_mod.o

all:	$(MODULES)

ppp_mod.o:	ppp.o ppp_vdcmd.o
	$(LD) -r -o ppp_mod.o ppp.o ppp_vdcmd.o

ppp_ahdl_mod.o: ppp_ahdlc.o ppp_ahdlc_vdcmd.o
	$(LD) -r -o ppp_ahdl_mod.o ppp_ahdlc.o ppp_ahdlc_vdcmd.o

COMP_OBJS = ppp_comp.o bsd-comp.o deflate.o zlib.o vjcompress.o \
	ppp_comp_vdcmd.o
ppp_comp_mod.o: $(COMP_OBJS)
	$(LD) -r -o $@ $(COMP_OBJS)

if_ppp.o: ../modules/if_ppp.c
	$(CC) $(CFLAGS) -c $?
bsd-comp.o: ../modules/bsd-comp.c
	$(CC) $(CFLAGS) -c $?
deflate.o: ../modules/deflate.c
	$(CC) $(CFLAGS) -c $?
ppp.o:	../modules/ppp.c
	$(CC) $(CFLAGS) -c $?
ppp_ahdlc.o: ../modules/ppp_ahdlc.c
	$(CC) $(CFLAGS) -c $?
ppp_comp.o: ../modules/ppp_comp.c
	$(CC) $(CFLAGS) -c $?
vjcompress.o: ../modules/vjcompress.c
	$(CC) $(CFLAGS) -c $?
zlib.o:	../common/zlib.c
	$(CC) $(CFLAGS) -c $?

if_ppp_mod.o:	if_ppp.o if_ppp_vdcmd.o
	$(LD) -r -o if_ppp_mod.o if_ppp.o if_ppp_vdcmd.o

install: all
	$(INSTALL) $(MODULES) $(BINDIR)
	./ppp.INSTALL

clean:
	rm -f ppp ppp_comp ppp_ahdl *.o *~ core
