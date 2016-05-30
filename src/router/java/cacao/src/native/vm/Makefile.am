## src/native/vm/Makefile.am
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

DIST_SUBDIRS = \
	cldc1.1 \
	gnuclasspath \
	openjdk

if WITH_JAVA_RUNTIME_LIBRARY_CLDC1_1
SUBDIRS = \
	cldc1.1

NATIVEVM_LIB = \
	cldc1.1/libnativevmcore.la
endif

if WITH_JAVA_RUNTIME_LIBRARY_GNU_CLASSPATH
SUBDIRS = \
	gnuclasspath

NATIVEVM_LIB = \
	gnuclasspath/libnativevmcore.la
endif

if WITH_JAVA_RUNTIME_LIBRARY_OPENJDK
SUBDIRS = \
	openjdk

NATIVEVM_LIB = \
	openjdk/libnativevmcore.la
endif

if ENABLE_JAVASE
REFLECTION_SOURCES = \
	reflection.cpp \
	reflection.hpp

SUN_MISC_UNSAFE_SOURCES = \
	sun_misc_Unsafe.cpp
endif

noinst_LTLIBRARIES = \
	libnativevm.la

libnativevm_la_SOURCES = \
	nativevm.cpp \
	nativevm.hpp \
	$(REFLECTION_SOURCES) \
	$(SUN_MISC_UNSAFE_SOURCES)

libnativevm_la_LIBADD = \
	$(NATIVEVM_LIB)


## Local variables:
## mode: Makefile
## indent-tabs-mode: t
## c-basic-offset: 4
## tab-width: 8
## compile-command: "automake --add-missing"
## End:
