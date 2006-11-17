ifneq ($(wildcard $(SRCBASE)/cy_conf.mak),)
  include $(SRCBASE)/cy_conf.mak
endif

include $(TOP)/.config

OBJS := ifenslave.o
CFLAGS := $(COPTS) -I$(LINUXDIR)/include 
  
all: ifenslave

ifenslave: $(OBJS)
	$(CC) -s -o $@ $^ $(LDFLAGS)

clean:
	rm -f *.o *~ $(UTILS)

install: all
	install -d $(INSTALLDIR)/usr/sbin
	install ifenslave $(INSTALLDIR)/usr/sbin
