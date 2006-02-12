
include $(TOP)/.config

ifneq ($(wildcard $(SRCBASE)/cy_conf.mak),)
  include $(SRCBASE)/cy_conf.mak
endif


CFLAGS	= -I$(TOP)/shared -I$(SRCBASE)/include -Wall -I$(SRCBASE)/
CFLAGS  += -I$(TOP)/iptables/include -I$(TOP)/libnet/include
CFLAGS	+= -s -Os -pipe -mips32 -mtune=mips32


OBJS := robocfg.o



vpath %.c $(TOP)/shared $(SRCBASE)/rts/src

all: robocfg

clean:
	rm -f *.o robocfg

install: all
	install -d $(INSTALLDIR)/usr/sbin
	install robocfg $(INSTALLDIR)/usr/sbin/robocfg
	$(STRIP) $(INSTALLDIR)/usr/sbin/robocfg

robocfg: $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

$(OBJS): $(CY_DEPS)
