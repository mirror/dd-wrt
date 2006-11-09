ifneq ($(wildcard $(SRCBASE)/cy_conf.mak),)
  include $(SRCBASE)/cy_conf.mak
endif

include $(TOP)/.config

OBJS := scdp.o
CFLAGS := $(COPTS) -I. -I../libnet/include 
ifeq ($(CONFIG_MAGICBOX),y)
    CFLAGS+= -DLIBNET_BIG_ENDIAN
else
    ifeq ($(CONFIG_XSCALE),y)
    CFLAGS+= -DLIBNET_BIG_ENDIAN
    else
	CFLAGS+= -DLIBNET_LIL_ENDIAN
    endif
endif
  
LDFLAGS := -L../libnet/lib

all: scdp

scdp: $(OBJS)
	$(CC) -s -o $@ $^ $(LDFLAGS) -lnet

clean:
	rm -f *.o *~ $(UTILS)

install: all
	install -d $(INSTALLDIR)/usr/sbin
	install scdp $(INSTALLDIR)/usr/sbin
