include $(TOP)/.config

TARGET  := mmc
WARN    := -W -Wall -Wstrict-prototypes -Wmissing-prototypes -fno-strict-aliasing -G 0 -Wa,--trap -Wa,-32 -Wa,-march=mips32 -Wa,-mips32 -fno-delayed-branch
INCLUDE := -isystem $(LINUXDIR)/include -I$(TOP)/../include.v23

CFLAGS  := -O2 -mtune=mips32 -mtune=mips32 -pipe -funit-at-a-time -DMODULE -D__KERNEL__ ${WARN} ${INCLUDE} -fno-pic -mno-abicalls -mlong-calls
CC      := /opt/10.2.0/bin/mipsel-linux-uclibc-gcc


all:
	$(CC) $(CFLAGS) -c -o mmc.o mmcold.c
#	$(CC) $(CFLAGS) -DTWO -c -o mmc_wrt2.o mmc.c
#	$(CC) $(CFLAGS) -DTHREE -c -o mmc_buf1.o mmc.c
#	$(CC) $(CFLAGS) -DFOUR -c -o mmc_buf2.o mmc.c

clean:
	rm -f mmc.o