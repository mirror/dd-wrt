include ../../.config
include ../../Config.mak

SRCS = ipsecadm.c sa.c tunnel.c stats.c key.c
OBJS = $(SRCS:.c=.o)

TARGET = ipsecadm

all: $(TARGET)

install: all
	$(INSTALL) -D -s ipsecadm $(SBINDIR)/ipsecadm
	$(INSTALL) -D -m 664 ipsecadm.8 $(MANDIR)/man8/ipsecadm.8

clean:
	rm -f $(OBJS) $(TARGET)

depend:
	$(CC) $(CFLAGS) -MM $(SRCS) > .depend

ipsecadm: $(OBJS)

ifneq ($(wildcard .depend),)
include .depend
endif
