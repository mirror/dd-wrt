# Makefile for uClibc
#
# Copyright (C) 2000,2001 Erik Andersen <andersen@uclibc.org>
#
# This program is free software; you can redistribute it and/or modify it under
# the terms of the GNU Library General Public License as published by the Free
# Software Foundation; either version 2 of the License, or (at your option) any
# later version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE. See the GNU Library General Public License for more
# details.
#
# You should have received a copy of the GNU Library General Public License
# along with this program; if not, write to the Free Software Foundation, Inc.,
# 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

TESTDIR=../
include $(TESTDIR)/Rules.mak



TARGETS=hello_source hello hello_glibc tiny

all: $(TARGETS)

hello_source:
	-@ $(RM) $(TARGETS)
	-@ echo "-------"
	-@ echo "hello.c source: "
	-@ echo " "
	-@ cat hello.c
	-@ echo " "

hello: hello.c Makefile $(TESTDIR)/Config $(TESTDIR)/Rules.mak
	-@ echo "-------"
	-@ echo " "
	-@ echo "Compiling vs uClibc: "
	-@ echo " "
	$(CC) $(CFLAGS) -c $< -o $@.o
	$(CC) $(LDFLAGS) $@.o -o $@ $(EXTRA_LIBS)
	$(STRIPTOOL) -x -R .note -R .comment $@
	-$(LDD) $@
	ls -l $@
	-./$@
	-@ echo " "

hello_glibc: hello.c Makefile
	-@ echo "-------"
	-@ echo " "
	-@ echo "Compiling vs GNU libc: "
	-@ echo " "
	$(HOSTCC) $(GLIBC_CFLAGS) -c $< -o $@.o
	$(HOSTCC) $(GLIBC_LDFLAGS) $@.o -o $@
	$(STRIPTOOL) -x -R .note -R .comment $@
	-$(LDD) $@
	ls -l $@
	-./$@
	-@ echo " "


tiny: tiny.c Makefile $(TESTDIR)/Config $(TESTDIR)/Rules.mak
	-@ echo "-------"
	-@ echo " "
	-@ echo "Compiling vs uClibc: "
	-@ echo " "
	$(CC) $(CFLAGS) -c $< -o $@.o
	$(CC) $(LDFLAGS) $@.o -o $@ $(EXTRA_LIBS)
	$(STRIPTOOL) -x -R .note -R .comment $@
	-$(LDD) $@
	ls -l $@
	-./$@
	-@ echo " "


clean:
	$(RM) *.[oa] *~ core $(TARGETS)


