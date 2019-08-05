obj-m := sch_fq_codel_fast.o
KERNEL_VERSION := $(shell cat $(LINUXDIR)/include/config/kernel.release 2> /dev/null)
IDIR := $(INSTALLDIR)/lib/modules/$(KERNEL_VERSION)/net/sched
KDIR := $(LINUXDIR)
PWD := $(shell pwd)
VERSION := 666
default:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules $(if $(VERSION),LDFLAGS_MODULE="--build-id=0x$(VERSION)" CFLAGS_MODULE="-DCAKE_VERSION=\\\"$(VERSION)\\\"")

install:
	install -v -m 644 -D sch_fq_codel_fast.ko $(IDIR)/sch_fq_codel_fast.ko

clean:
	rm -rf Module.markers modules.order Module.symvers sch_fq_codel_fast.ko sch_fq_codel_fast.mod.c sch_fq_codel_fast.mod.o sch_fq_codel_fast.o
