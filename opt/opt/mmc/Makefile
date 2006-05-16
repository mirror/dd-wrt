TARGET  := mmc
WARN    := -W -Wall -Wstrict-prototypes -Wmissing-prototypes -fno-strict-aliasing -G 0 -Wa,--trap -Wa,-32 -Wa,-march=mips32 -Wa,-mips32 -fno-delayed-branch
INCLUDE := -isystem ../../src/linux/linux/include 
CFLAGS  := -Os -pipe -mips32 -mtune=mips32 -DMODULE -D__KERNEL__ ${WARN} ${INCLUDE} -fno-pic -mno-abicalls -mlong-calls
CC      := mipsel-linux-uclibc-gcc

${TARGET}.o: ${TARGET}.c
