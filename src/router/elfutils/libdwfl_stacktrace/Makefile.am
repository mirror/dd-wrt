## Makefile.am for libdwfl_stacktrace library subdirectory in elfutils.
##
## Process this file with automake to create Makefile.in
##
## Copyright (C) 2025 Red Hat, Inc.
## This file is part of elfutils.
##
## This file is free software; you can redistribute it and/or modify
## it under the terms of either
##
##   * the GNU Lesser General Public License as published by the Free
##     Software Foundation; either version 3 of the License, or (at
##     your option) any later version
##
## or
##
##   * the GNU General Public License as published by the Free
##     Software Foundation; either version 2 of the License, or (at
##     your option) any later version
##
## or both in parallel, as here.
##
## elfutils is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
## General Public License for more details.
##
## You should have received copies of the GNU General Public License and
## the GNU Lesser General Public License along with this program.  If
## not, see <http://www.gnu.org/licenses/>.
##
include $(top_srcdir)/config/eu.am
AM_CPPFLAGS += -I$(srcdir) -I$(srcdir)/../libelf -I$(srcdir)/../libebl \
	   -I$(srcdir)/../libdw -I$(srcdir)/../libdwelf -I$(srcdir)/../libdwfl
VERSION = 1

noinst_LIBRARIES = libdwfl_stacktrace.a
noinst_LIBRARIES += libdwfl_stacktrace_pic.a

pkginclude_HEADERS = libdwfl_stacktrace.h


libdwfl_stacktrace_a_SOURCES = dwflst_process_tracker.c \
			       dwflst_tracker_find_elf.c \
			       dwflst_tracker_elftab.c \
			       dwflst_tracker_dwfltab.c \
			       libdwfl_stacktrace_next_prime.c \
			       dwflst_sample_frame.c

libdwfl_stacktrace = $(libdw)
libdw = ../libdw/libdw.so
libelf = ../libelf/libelf.so
libebl = ../libebl/libebl.a
libeu = ../lib/libeu.a

libdwfl_stacktrace_pic_a_SOURCES =
am_libdwfl_stacktrace_pic_a_OBJECTS = $(libdwfl_stacktrace_a_SOURCES:.c=.os)

noinst_HEADERS = libdwfl_stacktraceP.h \
		 dwflst_tracker_elftab.h dwflst_tracker_dwfltab.h

EXTRA_libdwfl_stacktrace_a_DEPENDENCIES = libdwfl_stacktrace.manifest

libdwfl_stacktrace.manifest: $(libdwfl_stacktrace_a_OBJECTS)
	$(AM_V_GEN)echo $^ > $@

MOSTLYCLEANFILES = $(am_libdwfl_stacktrace_a_OBJECTS) \
	$(am_libdwfl_stacktrace_pic_a_OBJECTS)
CLEANFILES = $(EXTRA_libdwfl_stacktrace_a_DEPENDENCIES)
