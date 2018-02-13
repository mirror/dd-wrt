src = $(TOP)/zfs/unicode
obj = $(TOP)/zfs/unicode

MODULE := zunicode

EXTRA_CFLAGS = $(ZFS_MODULE_CFLAGS)  -std=gnu99 -Wno-declaration-after-statement -Wno-unused-but-set-variable  -DHAVE_SPL -D_KERNEL -DTEXT_DOMAIN=\"zfs-linux-kernel\" -DNDEBUG 

obj-$(CONFIG_ZFS) := $(MODULE).o

$(MODULE)-objs += u8_textprep.o
$(MODULE)-objs += uconv.o
