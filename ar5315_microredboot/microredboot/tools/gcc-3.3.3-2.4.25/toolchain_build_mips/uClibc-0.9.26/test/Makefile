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

TOPDIR=../
-include $(TOPDIR).config

.EXPORT_ALL_VARIABLES:


ALL_SUBDIRS = args assert ctype dlopen pwd_grp signal silly stdlib string unistd crypt #misc
DIRS = $(ALL_SUBDIRS)
#ifeq ($(TARGET_ARCH), $(HOST_ARCH))
#	DIRS = $(ALL_SUBDIRS)
#else
#	DIRS =
#endif
ifeq ($(strip $(HAVE_SHARED)),true)
  ifeq ($(strip $(DODYNAMIC)),true)
	DIRS += dlopen
  endif	
endif	
ifeq ($(strip $(INCLUDE_THREADS)),true)
    DIRS += pthread
endif	

all: subdirs

tags:
	ctags -R
	
clean: subdirs_clean
	$(RM) *.[oa] *~ core

subdirs: $(patsubst %, _dir_%, $(DIRS))
subdirs_clean: $(patsubst %, _dirclean_%, $(ALL_SUBDIRS))

$(patsubst %, _dir_%, $(DIRS)) : dummy
	$(MAKE) -C $(patsubst _dir_%, %, $@)

$(patsubst %, _dirclean_%, $(ALL_SUBDIRS)) : dummy
	$(MAKE) -C $(patsubst _dirclean_%, %, $@) clean

.PHONY: dummy
