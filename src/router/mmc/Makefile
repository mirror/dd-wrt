include $(TOP)/.config

TARGET  := mmc
#WARN    := -W -Wall -Wstrict-prototypes -Wmissing-prototypes -fno-strict-aliasing -G 0 -Wa,--trap -Wa,-32 -Wa,-march=mips32 -Wa,-mips32 -fno-delayed-branch
INCLUDE := -isystem $(LINUXDIR)/include 

CFLAGS  := $(COPTS) -DMODULE -D__KERNEL__ ${INCLUDE} -fno-pic 

#-mno-abicalls -mlong-calls
#CC      := /opt/3.3.6/bin/mipsel-linux-uclibc-gcc

${TARGET}.o: ${TARGET}.c


clean:
	rm -f mmc.o