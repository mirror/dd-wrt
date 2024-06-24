##
## Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008, 2010, 2011, 2012, 2013
## Robert Lougher <rob@jamvm.org.uk>.
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

SUBDIRS = os interp classlib
DIST_SUBDIRS = os arch interp classlib

noinst_HEADERS = jni.h

lib_LTLIBRARIES = libjvm.la
noinst_LTLIBRARIES = libcore.la

libcore_la_SOURCES = alloc.c alloc.h cast.c class.c dll.c excep.c execute.c \
                     frame.h hash.c hash.h jam.h jni.c lock.c lock.h \
                     natives.c reflect.c resolve.c sig.h string.c thread.c \
                     thread.h utf8.c zip.c zip.h properties.c natives.h \
                     access.c frame.c init.c hooks.c class.h \
                     symbol.c symbol.h excep.h shutdown.c time.c reflect.h \
                     jni-internal.h properties.h sig.c stubs.h stubs.c \
                     jni-stubs.c annotations.h

libjvm_la_SOURCES =

libjvm_la_LIBADD = libcore.la
libjvm_la_LDFLAGS = -avoid-version
libcore_la_LIBADD = interp/libinterp.la os/@os@/@arch@/libnative.la \
                    os/@os@/libos.la classlib/@classlib@/libclasslib.la

AM_CPPFLAGS = -I$(top_srcdir)/src/interp/engine

DISTCLEANFILES = arch.h classlib.h classlib-defs.h classlib-symbol.h \
                 classlib-excep.h
