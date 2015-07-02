
include $(TOP)/.config

ifneq ($(wildcard $(SRCBASE)/cy_conf.mak),)
  include $(SRCBASE)/cy_conf.mak
endif

CFLAGS	+= -I. -I$(TOP)/shared -I$(SRCBASE)/include.v24 -Wall -I$(SRCBASE)/ $(COPTS) $(MIPS16_OPT) -D_GNU_SOURCE -ffunction-sections -fdata-sections -Wl,--gc-sections 

ap_client: ap_client.o
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm -f ap_client ap_client.o

all: ap_client


install:
	
	install -d $(INSTALLDIR)/usr/sbin
	cp ap_client $(INSTALLDIR)/usr/sbin
