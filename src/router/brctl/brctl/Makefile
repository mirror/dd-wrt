#
# $Id: Makefile,v 1.1 2005/09/28 11:53:38 seg Exp $

CFLAGS = -Wall $(COPTS) $(MIPS16_OPT) $(LTO) -I../libbridge -DNEED_PRINTF -ffunction-sections -fdata-sections -Wl,--gc-sections
#LDFLAGS = -g 
ifeq ($(strip $(DOSTATIC)),true)
    LDFLAGS += --static
endif

all:			brctl

brctl:			brctl.o brctl_cmd.o brctl_disp.o ../libbridge/libbridge.a
			$(CC) $(LDFLAGS) $(LDLTO) $(COPTS) -o brctl brctl.o brctl_cmd.o brctl_disp.o ../libbridge/libbridge.a

clean:
			rm -f *~
			rm -f *.o
			rm -f brctl
			rm -f brctld
			rm -f core

brctl.o:		brctl.c brctl.h
			$(CC) $(CFLAGS) -c -o brctl.o brctl.c

brctl_cmd.o:		brctl_cmd.c brctl.h
			$(CC) $(CFLAGS) -c -o brctl_cmd.o brctl_cmd.c

brctl_disp.o:		brctl_disp.c brctl.h
			$(CC) $(CFLAGS) -c -o brctl_disp.o brctl_disp.c
