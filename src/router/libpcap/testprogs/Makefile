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
EXTRA_NETWORK_LIBS=

# Standard CFLAGS for building test programs
FULL_CFLAGS = $(CCOPT) $(INCLS) $(DEFS) $(CFLAGS)

INSTALL = /usr/bin/install -c
INSTALL_PROGRAM = ${INSTALL}
INSTALL_DATA = ${INSTALL} -m 644

# Explicitly define compilation rule since SunOS 4's make doesn't like gcc.
# Also, gcc does not remove the .o before forking 'as', which can be a
# problem if you don't own the file but can write to the directory.
.c.o:
	@rm -f $@
	$(CC) $(FULL_CFLAGS) -c $(srcdir)/$*.c

SRC =  \
	can_set_rfmon_test.c \
	capturetest.c \
	filtertest.c \
	findalldevstest-perf.c \
	findalldevstest.c \
	opentest.c \
	reactivatetest.c \
	selpolltest.c \
	threadsignaltest.c \
	writecaptest.c

TESTS = $(SRC:.c=)

TAGFILES = \
	$(SRC) $(HDR)

CLEANFILES = $(OBJ) $(TESTS)

all: $(TESTS)

capturetest: $(srcdir)/capturetest.c ../libpcap.a
	$(CC) $(FULL_CFLAGS) -I. -L. -o capturetest $(srcdir)/capturetest.c ../libpcap.a $(LIBS)

can_set_rfmon_test: $(srcdir)/can_set_rfmon_test.c ../libpcap.a
	$(CC) $(FULL_CFLAGS) -I. -L. -o can_set_rfmon_test $(srcdir)/can_set_rfmon_test.c ../libpcap.a $(LIBS)

filtertest: $(srcdir)/filtertest.c ../libpcap.a
	$(CC) $(FULL_CFLAGS) -I. -L. -o filtertest $(srcdir)/filtertest.c ../libpcap.a $(EXTRA_NETWORK_LIBS) $(LIBS)

findalldevstest: $(srcdir)/findalldevstest.c ../libpcap.a
	$(CC) $(FULL_CFLAGS) -I. -L. -o findalldevstest $(srcdir)/findalldevstest.c ../libpcap.a $(EXTRA_NETWORK_LIBS) $(LIBS)

findalldevstest-perf: $(srcdir)/findalldevstest-perf.c ../libpcap.a
	$(CC) $(FULL_CFLAGS) -I. -L. -o findalldevstest-perf $(srcdir)/findalldevstest-perf.c ../libpcap.a $(EXTRA_NETWORK_LIBS) $(LIBS)

opentest: $(srcdir)/opentest.c ../libpcap.a
	$(CC) $(FULL_CFLAGS) -I. -L. -o opentest $(srcdir)/opentest.c ../libpcap.a $(LIBS)

reactivatetest: $(srcdir)/reactivatetest.c ../libpcap.a
	$(CC) $(FULL_CFLAGS) -I. -L. -o reactivatetest $(srcdir)/reactivatetest.c ../libpcap.a $(LIBS)

selpolltest: $(srcdir)/selpolltest.c ../libpcap.a
	$(CC) $(FULL_CFLAGS) -I. -L. -o selpolltest $(srcdir)/selpolltest.c ../libpcap.a $(LIBS)

threadsignaltest: $(srcdir)/threadsignaltest.c ../libpcap.a
	$(CC) $(FULL_CFLAGS) -I. -L. -o threadsignaltest $(srcdir)/threadsignaltest.c ../libpcap.a $(LIBS) $(PTHREAD_LIBS)

valgrindtest: $(srcdir)/valgrindtest.c ../libpcap.a
	$(CC) $(FULL_CFLAGS) -I. -L. -o valgrindtest $(srcdir)/valgrindtest.c ../libpcap.a $(LIBS)

writecaptest: $(srcdir)/writecaptest.c ../libpcap.a
	$(CC) $(FULL_CFLAGS) -I. -L. -o writecaptest $(srcdir)/writecaptest.c ../libpcap.a $(LIBS)

clean:
	rm -f $(CLEANFILES)
	rm -rf *.dSYM

distclean: clean
	rm -f Makefile config.cache config.log config.status \
	    config.h stamp-h stamp-h.in
	rm -rf autom4te.cache

install:

uninstall:

tags: $(TAGFILES)
	ctags -wtd $(TAGFILES)

depend:
	$(MKDEP) -c "$(CC)" -m "$(DEPENDENCY_CFLAG)" -s "$(srcdir)" $(CFLAGS) $(DEFS) $(INCLS) $(SRC)
