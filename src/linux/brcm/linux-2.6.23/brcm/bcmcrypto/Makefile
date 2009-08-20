#
# Assorted cryptographic algorithms
#
# Copyright (C) 2002 Broadcom Corporation
#
# $Id: Makefile,v 1.22.36.1 2008/10/31 05:41:00 Exp $
#

SRCBASE	:= ..

CC	:= gcc
CFLAGS	+= -I. -I$(SRCBASE)/include -Wall
CFLAGS	+= -g -DDEBUG -ffunction-sections
CFLAGS	+= -O2

#CC	:= cl
#CFLAGS	+= -I. -I../include -Ic:/tools/msdev/2600ddk/inc/crt
#LDFLAGS	+= -link -LIBPATH:c:/tools/msdev/2600ddk/lib/wxp/i386

PROGS	:= aes aeskeywrap passhash prf rc4 sha1 tkhash dh sha256

all: $(PROGS)

clean:
	rm -f *.o *.obj $(PROGS) *.exe

AESFLAGS := -DBCMAES_TEST
aes:	aes.c rijndael-alg-fst.c
	$(CC) $(CFLAGS) $(AESFLAGS) -o $@ $^ $(LDFLAGS)

aeskeywrap:	aeskeywrap.c aes.c rijndael-alg-fst.c
	$(CC) $(CFLAGS) -DBCMAESKEYWRAP_TEST -o $@ $^ $(LDFLAGS)


passhash:	passhash.c sha1.c prf.c
	$(CC) $(CFLAGS) -DBCMPASSHASH_TEST -o $@ $^ $(LDFLAGS)

prf:	prf.c hmac.c sha1.c md5.c
	$(CC) $(CFLAGS) -DBCMPRF_TEST -DBCMCCX -o $@ $^ $(LDFLAGS)

rc4:	rc4.c
	$(CC) $(CFLAGS) -DBCMRC4_TEST -o $@ $^ $(LDFLAGS)

sha1:	sha1.c
	$(CC) $(CFLAGS) -DBCMSHA1_TEST -o $@ $^ $(LDFLAGS)

sha256:	sha256.c
	$(CC) $(CFLAGS) -DBCMSHA256_TEST -o $@ $^ $(LDFLAGS)

tkhash:	tkhash.c
	$(CC) $(CFLAGS) -DBCMTKHASH_TEST -o $@ $^ $(LDFLAGS)

dh:	dh.c bn.c
	$(CC) $(CFLAGS) -DBCMDH_TEST -o $@ $^ $(LDFLAGS)

install:
	install -d $(INSTALLDIR)/usr/sbin
	install $(PROGS) $(INSTALLDIR)/usr/sbin
	$(STRIP) $(foreach file,$(PROGS),$(INSTALLDIR)/usr/sbin/$(file))
