
#CC?=gcc

CFLAGS += $(LTO) $(COPTS) $(MIPS16_OPT) -I$(TOP)/wolfssl -I$(SSLPATH)/include   -D_GNU_SOURCE  -ffunction-sections -fdata-sections -Wl,--gc-sections -DNEED_PRINTF $(THUMB)
# Run this with make LIBS=-lrt if you want to compile on kfreebsd
LIBS = -L$(TOP)/nvram -lnvram -L$(SSLPATH) -lcrypto
all: multicall

clean: distclean

distclean:
	rm -f mactelnet macping mactelnetd mndp multicall
	rm -f *.o

potclean:
	rm -f po/*.pot

dist: distclean potclean pot

install: all
	install -d $(INSTALLDIR)/usr/bin
	install -d $(INSTALLDIR)/usr/sbin
	install -D multicall $(INSTALLDIR)/usr/bin/mactelnet_multicall
	cd $(INSTALLDIR)/usr/bin && ln -sf mactelnet_multicall mactelnet
	cd $(INSTALLDIR)/usr/sbin && ln -sf ../bin/mactelnet_multicall mactelnetd
	cd $(INSTALLDIR)/usr/bin && ln -sf mactelnet_multicall macping
	cd $(INSTALLDIR)/usr/bin && ln -sf mactelnet_multicall mndp

	install -d $(INSTALLDIR)/etc/config
	install -m 600 -o root config/mactelnet.webservices $(INSTALLDIR)/etc/config
	install -m 600 -o root config/mactelnet.nvramconfig $(INSTALLDIR)/etc/config

install-docs:
	install -d $(DESTDIR)/usr/share/man/man1/
	install docs/*.1 $(DESTDIR)/usr/share/man/man1/

autologin.o: autologin.c autologin.h
	${CC} -Wall ${CFLAGS} -c autologin.c

users.o: users.c users.h
	${CC} -Wall ${CFLAGS} -DUSERSFILE='"/tmp/mactelnetd.users"' -c users.c

protocol.o: protocol.c protocol.h
	${CC} -Wall ${CFLAGS} -c protocol.c

interfaces.o: interfaces.c interfaces.h
	${CC} -Wall ${CFLAGS} -c interfaces.c

console.o: console.c
	${CC} -Wall ${CFLAGS} -c console.c

mtwei.o: mtwei.c
	${CC} -Wall ${CFLAGS} -c mtwei.c

touch:
	touch mactelnetd.c

multicall: touch config.h multicall.o mactelnet.o mactelnetd.o mndp.o macping.o mactelnet.h protocol.o console.o interfaces.o autologin.o users.o mtwei.o
	install -m 600 -o root config/mactelnet.webservices $(TOP)/httpd/ej_temp
	${CC} -Wall ${CFLAGS} ${LDFLAGS} $(LDLTO) -o multicall multicall.o mactelnet.o mndp.o mactelnetd.o macping.o protocol.o console.o interfaces.o autologin.o users.o mtwei.o ${LIBS}
