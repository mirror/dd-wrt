#
# $Id: Makefile,v 1.16 1999/09/16 21:44:50 beyssac Exp $
#

DISTFILES=	tunip.c defs.h Makefile pipsecd.conf.sample TODO README COPYRIGHT
CLEANFILES=	*.core *.o pipsecd

# Where your OpenSSL includes are located
INCDIR=../openssl/include

# Where your libcrypto is located
LIBDIR=../openssl

# Optional: use FreeBSD's libmd
#OPTLIB += -lmd
#OPTDEF += -DUSE_SYSTEM_HASH

# Optional: use ethertap device under Linux instead of userlink
#OPTDEF += -DUSE_ETHERTAP

pipsecd:	tunip.c defs.h
	$(CC) -Os -pipe -mips32 -mtune=mips32  -Wall -I$(INCDIR) $(OPTDEF) -o pipsecd tunip.c -L$(LIBDIR) $(OPTLIB) -lcrypto

install: pipsecd
	install pipsecd /usr/local/sbin/

tar:
	tar cfvz tunip.tar.gz $(DISTFILES)

dist:
	TMP=pipsec-`date +%Y%m%d`;			\
	mkdir $$TMP;					\
	cp -p $(DISTFILES) $$TMP; tar cfvz $$TMP.tar.gz $$TMP;	\
	cp $$TMP.tar.gz $$HOME/public_html/;
#	rm -f $$HOME/public_html/tunip.tar.gz.old;	\
#	rm -f $$HOME/public_html/tunip.tar.gz;		\
#	ln $$HOME/public_html/$$TMP.tar.gz $$HOME/public_html/tunip.tar.gz

clean:
	rm -f $(CLEANFILES)
