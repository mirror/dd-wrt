export CROSS_COMPILE = mipsel-linux-uclibc-
export KCROSS_COMPILE = mipsel-linux-

CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)ld
DBGFLAGS = -s
OPTFLAGS = -Os
CFLAGS = -Wall -I. $(DBGFLAGS) $(OPTFLAGS)
LDFLAGS = $(DBGFLAGS)

KCC = $(KCROSS_COMPILE)gcc
KLD = $(KCROSS_COMPILE)ld
INCLUDE = -I$(KDIR)/include -I$(KDIR)/include/asm/gcc -I$(CRYPTODIR) -I$(KDIR)/../../include
KCFLAGS = -D__KERNEL__ -Wall -Wstrict-prototypes -Wno-trigraphs -Os -fno-strict-aliasing -fno-common -fomit-frame-pointer -G 0 -mno-abicalls -fno-pic -pipe -mips2 -Wa,--trap -DMODULE -mlong-calls -fno-common -nostdinc -iwithprefix include $(MODVERFLAG) $(INCLUDE)

include $(KDIR)/.config
ifdef CONFIG_MODVERSIONS
MODVERFLAG = -DMODVERSIONS -include $(KDIR)/include/linux/modversions.h
endif

INSTALL = install
DEPMOD = /sbin/depmod

BINDIR = $(EXEC_PREFIX)/bin
SBINDIR = $(EXEC_PREFIX)/sbin
MANDIR = $(PREFIX)/share/man

.PHONY: all install clean depend
