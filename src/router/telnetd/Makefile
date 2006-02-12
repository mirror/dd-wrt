
include $(TOP)/.config

ifneq ($(wildcard $(SRCBASE)/cy_conf.mak),)
  include $(SRCBASE)/cy_conf.mak
endif


CFLAGS	+= -I../busybox/include -I$(TOP)/shared -I$(SRCBASE)/include -Wall -I$(SRCBASE)/
CFLAGS  += -I$(TOP)/iptables/include -I$(TOP)/iptables/include/libipq -I$(TOP)/libnet/include -DL_ENDIAN -DLIBNET_LIL_ENDIAN -DLIB1X_LIL_ENDIAN
CFLAGS	+= -s $(COPTS)
LDFLAGS	+= ../busybox/libbb/libbb.a -otelnetd

OBJS := telnetd.o



vpath %.c $(TOP)/shared $(SRCBASE)/rts/src

all: telnetd

clean:
	rm -f *.o telnetd

install: all
	install -d $(INSTALLDIR)/usr/sbin
	install telnetd $(INSTALLDIR)/usr/sbin/telnetd
	$(STRIP) $(INSTALLDIR)/usr/sbin/telnetd

telnetd: $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

$(OBJS): $(CY_DEPS)
