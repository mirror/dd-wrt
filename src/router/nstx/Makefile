#CC = 
CFLAGS = -Wall $(COPTS)
CFLAGS += -Wall -Werror -Wsign-compare 

NSTXD_SRCS = nstxd.c nstx_encode.c nstx_pstack.c nstx_dns.c nstx_tuntap.c nstx_queue.c
NSTXD_OBJS = ${NSTXD_SRCS:.c=.o}

NSTXCD_SRCS = nstxcd.c nstx_encode.c nstx_pstack.c nstx_dns.c nstx_tuntap.o nstx_queue.c
NSTXCD_OBJS = ${NSTXCD_SRCS:.c=.o}

PROGS = nstxd nstxcd

all: $(PROGS)

nstxd: $(NSTXD_OBJS)
	$(CC) $(CFLAGS) -o nstxd $(NSTXD_OBJS)

nstxcd: $(NSTXCD_OBJS)
	$(CC) $(CFLAGS) -o nstxcd $(NSTXCD_OBJS)
	
install:
	install -d $(INSTALLDIR)/usr/sbin
	install -d $(INSTALLDIR)/etc/config
	install $(PROGS) $(INSTALLDIR)/usr/sbin
	install nstx.nvramconfig nstx.webservices $(INSTALLDIR)/etc/config

clean:
	rm -f *.o $(PROGS) Makefile.bak *~

