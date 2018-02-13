src = $(TOP)/zfs/zcommon
obj = $(TOP)/zfs/zcommon

MODULE := zcommon

EXTRA_CFLAGS = $(ZFS_MODULE_CFLAGS)  -std=gnu99 -Wno-declaration-after-statement -Wno-unused-but-set-variable  -DHAVE_SPL -D_KERNEL -DTEXT_DOMAIN=\"zfs-linux-kernel\" -DNDEBUG 

obj-$(CONFIG_ZFS) := $(MODULE).o

$(MODULE)-objs += zfs_deleg.o
$(MODULE)-objs += zfs_prop.o
$(MODULE)-objs += zprop_common.o
$(MODULE)-objs += zfs_namecheck.o
$(MODULE)-objs += zfs_comutil.o
$(MODULE)-objs += zfs_fletcher.o
$(MODULE)-objs += zfs_uio.o
$(MODULE)-objs += zpool_prop.o
$(MODULE)-objs += zfs_fletcher_superscalar.o
$(MODULE)-objs += zfs_fletcher_superscalar4.o

$(MODULE)-$(CONFIG_X86) += zfs_fletcher_intel.o
$(MODULE)-$(CONFIG_X86) += zfs_fletcher_sse.o
$(MODULE)-$(CONFIG_X86) += zfs_fletcher_avx512.o
$(MODULE)-$(CONFIG_ARM64) += zfs_fletcher_aarch64_neon.o
