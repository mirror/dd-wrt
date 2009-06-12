# Makefile for uClibc
#
# Copyright (C) 2000-2003 Erik Andersen <andersen@uclibc.org>
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

TOPDIR=../../../../
include $(TOPDIR)Rules.mak
ASFLAGS=$(CFLAGS)

CRT0_SRC = crt0.S
CRT0_OBJ = crt0.o crt1.o gcrt1.o
CRT0_DEPS=gmon-start.S
CTOR_TARGETS=$(TOPDIR)lib/crti.o $(TOPDIR)lib/crtn.o

SSRC=__longjmp.S setjmp.S bsd-setjmp.S bsd-_setjmp.S brk.S \
	clone.S __uClibc_syscall.S syscall.S
ifeq ($(strip $(UCLIBC_PROFILING)),y)
SSRC+=mcount.S
endif
SOBJS=$(patsubst %.S,%.o, $(SSRC))

CSRC=_mmap.c vfork.c __syscall_error.c pread_write.c ioctl.c
COBJS=$(patsubst %.c,%.o, $(CSRC))

OBJS=$(SOBJS) $(COBJS)

all: $(OBJS) $(LIBC)

$(LIBC): ar-target 

ar-target: $(OBJS) $(CRT0_OBJ) $(CTOR_TARGETS)
	$(AR) $(ARFLAGS) $(LIBC) $(OBJS)
	cp $(CRT0_OBJ) $(TOPDIR)lib/

$(CRT0_OBJ): $(CRT0_SRC)
	$(CC) $(CFLAGS) -DL_$* $< -c -o $*.o
	$(STRIPTOOL) -x -R .note -R .comment $*.o

$(SOBJS): %.o : %.S
	$(CC) $(CFLAGS) -c $< -o $@
	$(STRIPTOOL) -x -R .note -R .comment $*.o

$(COBJS): %.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@
	$(STRIPTOOL) -x -R .note -R .comment $*.o

ifeq ($(strip $(UCLIBC_PROFILING)),y)
gmon-start.S: ../common/gmon-start.c
	$(CC) $(CFLAGS) -c $< -S -o $*.S
gcrt1.o: $(CRT0_DEPS)
endif


ifeq ($(strip $(UCLIBC_CTOR_DTOR)),y)
crti.o: crti.S
	$(CC) $(CFLAGS) -c crti.S -o crti.o

$(TOPDIR)lib/crti.o: crti.o
	$(INSTALL) -d $(TOPDIR)lib/
	cp crti.o $(TOPDIR)lib/

crtn.o: crtn.S
	$(CC) $(CFLAGS) -c crtn.S -o crtn.o

$(TOPDIR)lib/crtn.o: crtn.o
	$(INSTALL) -d $(TOPDIR)lib/
	cp crtn.o $(TOPDIR)lib/
else
$(TOPDIR)lib/crti.o:
	$(INSTALL) -d $(TOPDIR)lib/
	$(AR) $(ARFLAGS) $(TOPDIR)lib/crti.o
$(TOPDIR)lib/crtn.o:
	$(INSTALL) -d $(TOPDIR)lib/
	$(AR) $(ARFLAGS) $(TOPDIR)lib/crtn.o
endif


headers:
	$(LN) -fs ../libc/sysdeps/linux/powerpc/fpu_control.h $(TOPDIR)/include/

clean:
	$(RM) *.[oa] *~ core
	$(RM) bits/sysnum.h
	$(RM) gmon-start.S

