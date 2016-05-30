## src/native/Makefile.am
##
## Copyright (C) 1996-2013
## CACAOVM - Verein zur Foerderung der freien virtuellen Maschine CACAO
##
## This file is part of CACAO.
##
## This program is free software; you can redistribute it and/or
## modify it under the terms of the GNU General Public License as
## published by the Free Software Foundation; either version 2, or (at
## your option) any later version.
##
## This program is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
## General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program; if not, write to the Free Software
## Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
## 02110-1301, USA.


AM_CPPFLAGS += -I$(top_srcdir)/src -I$(top_srcdir)/src/vm/jit/$(ARCH_DIR) -I$(top_srcdir)/src/vm/jit/$(ARCH_DIR)/$(OS_DIR) -I$(top_builddir)/src

LIBS =

SUBDIRS = \
	vm

if ENABLE_JVMTI
SUBDIRS += jvmti
JVMTI_LIB = jvmti/libjvmti.la
endif

noinst_LTLIBRARIES = libnative.la

if ENABLE_JNI
JNI_SOURCES = \
	jni.cpp \
	jni.hpp \
	localref.cpp \
	localref.hpp
endif

libnative_la_SOURCES = \
	$(JNI_SOURCES) \
	llni.cpp \
	llni.hpp \
	native.cpp \
	native.hpp

libnative_la_LIBADD = \
	vm/libnativevm.la \
	$(JVMTI_LIB)


## Local variables:
## mode: Makefile
## indent-tabs-mode: t
## c-basic-offset: 4
## tab-width: 8
## compile-command: "automake --add-missing"
## End:
