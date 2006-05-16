include ../.config
include ../Config.mak

CC = $(KCC)
LD = $(KLD)
CFLAGS = $(KCFLAGS)

SRCS = ipsec_sa.c ipsec_dev.c
OBJS = $(SRCS:.c=.o)

KVER := $(shell sh kversion.sh $(KDIR))
MODDIR = /lib/modules/$(KVER)

TARGET = ipsec_tunnel.o

all: $(TARGET)

install: all
	$(INSTALL) -D -m 664 $(TARGET) $(MODDIR)/misc/$(TARGET)
	$(DEPMOD) -a $(KVER)

clean:
	rm -f $(OBJS) $(TARGET)

depend:
	$(CC) $(KCFLAGS) -MM $(SRCS) > .depend

ipsec_tunnel.o: ipsec_dev.o ipsec_sa.o
	$(LD) -r $^ -o $@

ifneq ($(wildcard .depend),)
include .depend
endif
