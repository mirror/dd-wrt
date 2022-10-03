KERNEL_VERSION := $(shell cat $(LINUXDIR)/include/config/kernel.release 2> /dev/null)
IDIR := $(INSTALLDIR)/lib/modules/$(KERNEL_VERSION)
MODULES_DIR := $(IDIR)/kernel/net/netfilter/
KERNEL_DIR := $(LINUXDIR)

EXTRA_CFLAGS := -I$(src)/../include -DbKERNELMOD -Wimplicit-fallthrough=3


obj-m += xt_WGOBFS.o
xt_WGOBFS-objs += xt_WGOBFS_main.o siphash.o

all:
	make -C $(KERNEL_DIR) M=$(PWD) modules
clean:
	make -C ${KERNEL_DIR} M=$$PWD $@;
install:
	mkdir -p $(MODULES_DIR)
	cp xt_WGOBFS.ko $(MODULES_DIR)

