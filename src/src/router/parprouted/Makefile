EXTRA_CFLAGS =

#CC = gcc

CFLAGS = -g -Os -pipe -mips32 -mtune=mips32  -Wall $(EXTRA_CFLAGS)
LDFLAGS = -s 
OBJS = parprouted.o arp.o

LIBS = -lpthread

all: parprouted

install: all
	install parprouted /usr/local/sbin
	install parprouted.8 /usr/local/man/man8

clean:
	rm -f $(OBJS) parprouted core

parprouted:	${OBJS}
	${CC} -g -o parprouted ${OBJS} ${CFLAGS} ${LDFLAGS} ${LIBS}

parprouted.8:	parprouted.pod
	pod2man --section=8 --center="Proxy ARP Bridging Daemon" parprouted.pod --release "parprouted" --date "`date '+%B %Y'`" > parprouted.8
