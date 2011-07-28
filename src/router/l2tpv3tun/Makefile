LDFLAGS=		-lnl

CFLAGS=			-g -I. -MMD \
			-Wall -Werror -DL2TP_CMD_ROOT=\"l2tpv3tun\"

INSTALL?=	install
PROGS=		l2tpv3tun

all:	$(PROGS)

l2tpv3tun:	main.o utils.o
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	$(RM) $(wildcard *.o) $(wildcard *.d) $(PROGS)

install:
	$(INSTALL) -d $(DESTDIR)/usr/bin
	$(INSTALL) $(PROGS) $(DESTDIR)/usr/bin

sinclude $(wildcard *.d)
