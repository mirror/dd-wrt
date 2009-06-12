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

CSRC=	waitpid.c getdnnm.c gethstnm.c getcwd.c ptrace.c \
	mkfifo.c setegid.c wait.c getpagesize.c seteuid.c \
	wait3.c setpgrp.c getdtablesize.c create_module.c \
	cmsg_nxthdr.c longjmp.c open64.c ftruncate64.c mmap64.c \
	truncate64.c getrlimit64.c setrlimit64.c creat64.c \
	llseek.c pread_write.c _exit.c sync.c getdirname.c \
	sendfile64.c xstatconv.c getdents.c getdents64.c vfork.c \
	ulimit.c ntp_gettime.c
ifneq ($(strip $(EXCLUDE_BRK)),y)
CSRC+=sbrk.c
endif
ifeq ($(strip $(UCLIBC_PROFILING)),y)
CSRC+=gmon.c
endif
ifeq ($(strip $(UCLIBC_PROPOLICE)),y)
CSRC+=ssp.c
endif
COBJS=$(patsubst %.c,%.o, $(CSRC))

MSRC=syscalls.c
MOBJ=$(shell ./list_syscalls.sh)

OBJ=$(COBJS) $(MOBJ)

all: $(STR_SYSCALLS) $(OBJ) $(LIBC)

$(LIBC): ar-target

ar-target: $(OBJ)
	$(AR) $(ARFLAGS) $(LIBC) $(OBJ)

$(MOBJ): $(MSRC)
	$(CC) $(CFLAGS) -DL_$* $< -c -o $*.o
	$(STRIPTOOL) -x -R .note -R .comment $*.o

$(COBJS): %.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@
	$(STRIPTOOL) -x -R .note -R .comment $*.o

headers:
	$(LN) -fs ../libc/sysdeps/linux/common/fpu_control.h $(TOPDIR)/include/

clean:
	$(RM) *.[oa] *~ core crt[in].* *.S
