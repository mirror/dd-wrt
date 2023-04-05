#  Copyright (c) 1993, 1994, 1995, 1996
# 	The Regents of the University of California.  All rights reserved.
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that: (1) source code distributions
#  retain the above copyright notice and this paragraph in its entirety, (2)
#  distributions including binary code include the above copyright notice and
#  this paragraph in its entirety in the documentation or other materials
#  provided with the distribution, and (3) all advertising materials mentioning
#  features or use of this software display the following acknowledgement:
#  ``This product includes software developed by the University of California,
#  Lawrence Berkeley Laboratory and its contributors.'' Neither the name of
#  the University nor the names of its contributors may be used to endorse
#  or promote products derived from this software without specific prior
#  written permission.
#  THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
#  WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
#  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.

#
# Various configurable paths (remember to edit Makefile.in, not Makefile)
#

# Top level hierarchy
prefix = /usr
exec_prefix = /usr
datarootdir = ${prefix}/share
# Pathname of directory to install the configure program
bindir = /usr/bin
# Pathname of directory to install the rpcapd daemon
sbindir = /usr/sbin
# Pathname of directory to install the include files
includedir = ${prefix}/include
# Pathname of directory to install the library
libdir =  /usr/lib
# Pathname of directory to install the man pages
mandir = /usr/man

# VPATH
srcdir = .
top_srcdir = ..


#
# You shouldn't need to edit anything below.
#

LD = /usr/bin/ld
CC = ccache mips-linux-uclibc-gcc
AR = mips-linux-uclibc-ar
LN_S = ln -s
MKDEP = 
CCOPT =  -fvisibility=hidden -Oz -fno-unwind-tables -fno-asynchronous-unwind-tables -pipe -mips32r2 -mtune=34kc -msoft-float -fno-caller-saves  -mno-branch-likely -fno-unwind-tables -fno-asynchronous-unwind-tables -falign-jumps=1 -falign-labels=1 -falign-loops=1 -falign-functions=1 -fno-builtin-strlen -fno-guess-branch-probability -funsigned-char -finline-limit=0 -fno-builtin-printf -minterlink-mips16 -mips16 -std=c99 -D_GNU_SOURCE -fPIC
INCLS = -I. -I.. -I. -I./.. 
DEFS = -DHAVE_CONFIG_H 
ADDLOBJS = 
ADDLARCHIVEOBJS = 
LIBS = 
PTHREAD_LIBS = 
CROSSFLAGS=
CFLAGS = -Oz -fno-unwind-tables -fno-asynchronous-unwind-tables -pipe -mips32r2 -mtune=34kc -msoft-float -fno-caller-saves  -mno-branch-likely -fno-unwind-tables -fno-asynchronous-unwind-tables -falign-jumps=1 -falign-labels=1 -falign-loops=1 -falign-functions=1 -fno-builtin-strlen -fno-guess-branch-probability -funsigned-char -finline-limit=0 -fno-builtin-printf -minterlink-mips16 -mips16 -std=c99 -D_GNU_SOURCE -fPIC   ${CROSSFLAGS}
LDFLAGS =  ${CROSSFLAGS}
DYEXT = so
V_RPATH_OPT = -Wl,-rpath,
DEPENDENCY_CFLAG = 
PROG=libpcap
RPCAPD_LIBS = 

# Standard CFLAGS
FULL_CFLAGS = $(CCOPT)  $(INCLS) $(DEFS) $(CFLAGS)

INSTALL = /usr/bin/install -c
INSTALL_PROGRAM = ${INSTALL}
INSTALL_DATA = ${INSTALL} -m 644

# Explicitly define compilation rule since SunOS 4's make doesn't like gcc.
# Also, gcc does not remove the .o before forking 'as', which can be a
# problem if you don't own the file but can write to the directory.
.c.o:
	@rm -f $@
	$(CC) $(FULL_CFLAGS) -c $(srcdir)/$*.c

SRC =	daemon.c \
	fileconf.c \
	log.c \
	rpcapd.c

OBJ =	$(SRC:.c=.o) ../rpcap-protocol.o ../sockutils.o ../fmtutils.o ../sslutils.o
PUBHDR =

HDR = $(PUBHDR) log.h

TAGFILES = \
	$(SRC) $(HDR)

CLEANFILES = $(OBJ) rpcapd

MANADMIN = \
	rpcapd.manadmin.in

MANFILE = \
	rpcapd-config.manfile.in

rpcapd: $(OBJ) ../libpcap.a
	$(CC) $(CCOPT) $(CFLAGS) $(LDFLAGS)  \
	    -o $@ $(OBJ) ../libpcap.a $(LIBS) $(RPCAPD_LIBS) $(PTHREAD_LIBS)
clean:
	rm -f $(CLEANFILES)

distclean: clean
	rm -f Makefile config.cache config.log config.status \
	    config.h stamp-h stamp-h.in
	rm -f $(MANADMIN:.in=) $(MANFILE:.in=)
	rm -rf autom4te.cache

install: rpcapd
	[ -d $(DESTDIR)$(sbindir) ] || \
	    (mkdir -p $(DESTDIR)$(sbindir); chmod 755 $(DESTDIR)$(sbindir))
	$(INSTALL_PROGRAM) rpcapd $(DESTDIR)$(sbindir)/rpcapd
	[ -d $(DESTDIR)$(mandir)/man8 ] || \
	    (mkdir -p $(DESTDIR)$(mandir)/man8; chmod 755 $(DESTDIR)$(mandir)/man8)
	[ -d $(DESTDIR)$(mandir)/man5 ] || \
	    (mkdir -p $(DESTDIR)$(mandir)/man5; chmod 755 $(DESTDIR)$(mandir)/man5)
	for i in $(MANADMIN); do \
		$(INSTALL_DATA) `echo $$i | sed 's/.manadmin.in/.manadmin/'` \
		    $(DESTDIR)$(mandir)/man8/`echo $$i | sed 's/.manadmin.in/.8/'`; done
	for i in $(MANFILE); do \
		$(INSTALL_DATA) `echo $$i | sed 's/.manfile.in/.manfile/'` \
		    $(DESTDIR)$(mandir)/man5/`echo $$i | sed 's/.manfile.in/.5/'`; done

uninstall:
	rm -f $(DESTDIR)$(sbindir)/rpcapd
	for i in $(MANADMIN); do \
		rm -f $(DESTDIR)$(mandir)/man8/`echo $$i | sed 's/.manadmin.in/.8/'`; done
	for i in $(MANFILE); do \
		rm -f $(DESTDIR)$(mandir)/man5/`echo $$i | sed 's/.manfile.in/.5/'`; done

tags: $(TAGFILES)
	ctags -wtd $(TAGFILES)

depend:
	$(MKDEP) -c "$(CC)" -m "$(DEPENDENCY_CFLAG)" -s "$(srcdir)" $(CFLAGS) $(DEFS) $(INCLS) $(SRC)
