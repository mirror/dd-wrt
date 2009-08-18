
snoop:
	$(MAKE) -C $(LINUXDIR) M=$(TOP)/snoop modules

snoop-install: snoop.ko
	install -D $(TOP)/snoop/snoop.ko $(INSTALLDIR)/snoop/lib/modules/$(KERNELRELEASE)/snoop.ko

snoop-all: snoop

snoop.c: snoop.h

snoop-clean:
	cd $(TOP)/snoop && rm -f .*.cmd *~ *.o *.ko *.mod.* *.symvers
	cd $(TOP)/snoop && rm -rf *.tmp* .tmp*


