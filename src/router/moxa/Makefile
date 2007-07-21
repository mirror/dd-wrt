

##############################################################
ifeq	"$(findstring 2.6, $(shell uname -a))" "2.6"

all: module npreal2d tools
SP1: module npreal2d tools

npreal2d: 	npreal2d.o
	$(CC)	npreal2d.o -o npreal2d

npreal2d.o:	npreal2d.c
	$(CC) -c npreal2d.c

ifneq ($(KERNELRELEASE),)
obj-m := npreal2.o
else
KDIR	:= /lib/modules/$(shell uname -r)/build
PWD	:= $(shell pwd)

module:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules
	cp -p npreal2.ko /lib/modules/$(shell uname -r)/kernel/drivers/char/
	cp -p npreal2.ko /lib/modules/$(shell uname -r)/misc/
	depmod -a

endif

install:
	install  -d $(INSTALLDIR)/usr/sbin/
	install  mxaddsvr $(INSTALLDIR)/usr/sbin/
	install  mxdelsvr $(INSTALLDIR)/usr/sbin/
	install  mxcfmat $(INSTALLDIR)/usr/sbin/
	install  mxloadsvr $(INSTALLDIR)/usr/sbin/
	install  mxmknod $(INSTALLDIR)/usr/sbin/
	install  mxrmnod $(INSTALLDIR)/usr/sbin/
	install  npreal2d $(INSTALLDIR)/usr/sbin/

tools: mxaddsvr mxdelsvr mxcfmat mxloadsvr npreal2d

mxaddsvr: mxaddsvr.c
	$(CC) -o mxaddsvr mxaddsvr.c

mxdelsvr: mxdelsvr.c
	$(CC) -o mxdelsvr mxdelsvr.c

mxcfmat: mxcfmat.c
	$(CC) -o mxcfmat mxcfmat.c

mxloadsvr: mxloadsvr.c
	$(CC) -o mxloadsvr mxloadsvr.c
	
clean:
	rm -f *.o
	rm -rf ./.tmp_versions
	rm -f npreal2.mod*
	rm -f .npreal2*
	rm -f npreal2.ko
	rm -f npreal2d
	rm -f /lib/modules/$(shell uname -r)/kernel/drivers/char/npreal2.ko
	rm -f /lib/modules/$(shell uname -r)/misc/npreal2.ko
	rm -f mxaddsvr
	rm -f mxdelsvr
	rm -f mxcfmat
	rm -f mxloadsvr
	

else      #Linux 2.4########################################

CC=cc -DLINUX
ARCH=$(shell uname -m | sed s/i.86/i86/)
VERSION=$(shell uname -r | sed s/smp//)

ifeq	($(ARCH),i86)
OPTS=-DMODVERSIONS -DMODULE -D__KERNEL__ -Wall -Wstrict-prototypes -O2 \
-fomit-frame-pointer -pipe -march=i486 \
-I/usr/src/linux-$(VERSION)/include -I/usr/src/linux-2.4/include -I/usr/src/linux/include
else
OPTS=-DMODVERSIONS -DMODULE -D__KERNEL__ -Wall -Wstrict-prototypes -O2 \
-fomit-frame-pointer -pipe -I/usr/src/linux/include -ffixed-8
endif


ifeq	"$(findstring SMP, $(shell uname -a))" "SMP"
OPTS+=-D__SMP__
endif



all: module npreal2d tools

SP1: modulesp1 npreal2d tools


npreal2d: 	npreal2d.o
	cc	npreal2d.o -o npreal2d
	strip	npreal2d

npreal2d.o:	npreal2d.c
	$(CC) -c npreal2d.c

module:
	$(CC) -c $(OPTS) npreal2.c
	cp -p npreal2.o /lib/modules/$(shell uname -r)/kernel/drivers/char/
	cp -p npreal2.o /lib/modules/$(shell uname -r)/misc/
	depmod -a

modulesp1:
	$(CC) -c $(OPTS) -DSP1 npreal2.c
	cp -p npreal2.o /lib/modules/$(shell uname -r)/kernel/drivers/char/
	cp -p npreal2.o /lib/modules/$(shell uname -r)/misc/
	depmod -a
	
tools: mxaddsvr mxdelsvr mxcfmat mxloadsvr

mxaddsvr: mxaddsvr.c
	$(CC) -o mxaddsvr mxaddsvr.c
	strip mxaddsvr

mxdelsvr: mxdelsvr.c
	$(CC) -o mxdelsvr mxdelsvr.c
	strip mxdelsvr

mxcfmat: mxcfmat.c
	$(CC) -o mxcfmat mxcfmat.c
	strip mxcfmat

mxloadsvr: mxloadsvr.c
	$(CC) -o mxloadsvr mxloadsvr.c
	strip mxloadsvr

clean:
	rm -rf *.o
	rm -f npreal2d
	rm -f /lib/modules/$(shell uname -r)/kernel/drivers/char/npreal2.ko
	rm -f /lib/modules/$(shell uname -r)/misc/npreal2.ko
	rm -f mxaddsvr
	rm -f mxdelsvr
	rm -f mxcfmat
	rm -f mxloadsvr

endif
