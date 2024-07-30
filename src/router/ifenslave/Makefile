ifneq ($(wildcard $(SRCBASE)/cy_conf.mak),)
  include $(SRCBASE)/cy_conf.mak
endif

include $(TOP)/.config

OBJS := ifenslave.o
CFLAGS := $(COPTS) $(MIPS16_OPT) $(LTO) -I$(TOP)/kernel_headers/$(KERNELRELEASE)/include -I$(LINUXDIR)/include 
  
all: ifenslave

ifenslave: $(OBJS)
	$(CC) $(COPTS) -s -o $@ $^ $(LDFLAGS)

clean:
	rm -f *.o ifenslave *~ $(UTILS)

install: all
	install -d $(INSTALLDIR)/usr/sbin
	install ifenslave $(INSTALLDIR)/usr/sbin
