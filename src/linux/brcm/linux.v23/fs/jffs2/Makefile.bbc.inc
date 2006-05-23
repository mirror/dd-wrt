JFFS2_BBC_KERNEL_OBJS-y = jffs2_bbc_framework.o jffs2_bbc_fs.o

JFFS2_BBC_KERNEL_OBJS-$(CONFIG_JFFS2_BBC_ARMLIB) += jffs2_bbc_armlib_comp.o
JFFS2_BBC_KERNEL_OBJS-$(CONFIG_JFFS2_BBC_LZO) += jffs2_bbc_lzo_comp.o
JFFS2_BBC_KERNEL_OBJS-$(CONFIG_JFFS2_BBC_LZSS) += jffs2_bbc_lzss_comp.o
JFFS2_BBC_KERNEL_OBJS-$(CONFIG_JFFS2_BBC_LZARI) += jffs2_bbc_lzari_comp.o
JFFS2_BBC_KERNEL_OBJS-$(CONFIG_JFFS2_BBC_LZHD) += jffs2_bbc_lzhd_comp.o

JFFS2_BBC_KERNEL_OBJS := $(JFFS2_BBC_KERNEL_OBJS-y)

JFFS2_BBC_MKFS_OBJS   = jffs2_bbc_mkfs.o jffs2_bbc_framework.o jffs2_bbc_armlib_comp.o jffs2_bbc_lzo_comp.o\
                        jffs2_bbc_lzss_comp.o jffs2_bbc_lzari_comp.o jffs2_bbc_lzhd_comp.o
