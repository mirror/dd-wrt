##
## Copyright (C) 2010, 2011, 2012, 2013 Robert Lougher <rob@jamvm.org.uk>.
##
## This file is part of JamVM.
##
## This program is free software; you can redistribute it and/or
## modify it under the terms of the GNU General Public License
## as published by the Free Software Foundation; either version 2,
## or (at your option) any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program; if not, write to the Free Software
## Foundation, 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
##

noinst_LTLIBRARIES = libclasslib.la
libclasslib_la_SOURCES = thread.c class.c natives.c excep.c reflect.c \
                         dll.c jni.c jvm.c properties.c openjdk.h \
                         trace.h management.c jmm.h access.c frame.c \
                         classlib.h classlib-symbol.h classlib-defs.h \
                         shutdown.c alloc.c perf.c mh.c classlib-excep.h

AM_CPPFLAGS = -I$(top_builddir)/src -I$(top_srcdir)/src
