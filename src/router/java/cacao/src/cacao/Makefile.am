## src/cacao/Makefile.am
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


AM_CPPFLAGS += -I$(top_srcdir)/src -I$(top_srcdir)/src/vm/jit/$(ARCH_DIR) -I$(top_srcdir)/src/vm/jit/$(ARCH_DIR)/$(OS_DIR)

if ENABLE_STATICVM
cacao_LDFLAGS = \
	-all-static
else
cacao_LDFLAGS =
endif

if ENABLE_LIBJVM
lib_LTLIBRARIES = \
	libjvm.la

libjvm_la_LDFLAGS = \
	-avoid-version

if WITH_JAVA_RUNTIME_LIBRARY_OPENJDK
if WITH_GNU_LD
libjvm_la_LDFLAGS += \
	-Xlinker --version-script=$(top_srcdir)/contrib/mapfile-vers-product
else
# This is for the Solaris LD.
libjvm_la_LDFLAGS += \
	-Xlinker -M$(top_srcdir)/contrib/mapfile-vers-product
endif
endif

if GCC_PR40134
libjvm_la_LDFLAGS += -lgcc
endif

cacao_LDFLAGS += \
	-export-dynamic

CACAO_LIBS =
else
noinst_LTLIBRARIES = \
	libjvm.la

cacao_LDFLAGS += \
	-export-dynamic

CACAO_LIBS = \
	libjvm.la
endif

if ENABLE_RT_TIMING
cacao_LDFLAGS += -lrt
endif

libjvm_la_SOURCES =

# Dummy C++ source to cause C++ linking.
# http://www.gnu.org/software/automake/manual/automake.html#Libtool-Convenience-Libraries
nodist_EXTRA_libjvm_la_SOURCES = \
	dummy.cxx

libjvm_la_LIBADD = \
	$(top_builddir)/src/fdlibm/libfdlibm.la \
	$(top_builddir)/src/future/libfuture.la \
	$(top_builddir)/src/mm/libmm.la \
	$(top_builddir)/src/native/libnative.la \
	$(top_builddir)/src/threads/libthreads.la \
	$(top_builddir)/src/toolbox/libtoolbox.la \
	$(top_builddir)/src/vm/libvm.la \
	$(GC_LIB)

bin_PROGRAMS = \
	cacao

cacao_SOURCES = \
	cacao.cpp

cacao_LDADD = \
	$(CACAO_LIBS)

cacao_DEPENDENCIES = \
	$(cacao_LDADD)


if WITH_JAVA_RUNTIME_LIBRARY_OPENJDK
install-data-local:
	$(mkdir_p) $(prefix)/jre/lib/$(JAVA_ARCH)/server
	$(LN_S) -f $(libdir)/libjvm.so $(prefix)/jre/lib/$(JAVA_ARCH)/server

uninstall-local:
	rm -f $(prefix)/jre/lib/$(JAVA_ARCH)/server/libjvm.so
endif


## Local variables:
## mode: Makefile
## indent-tabs-mode: t
## c-basic-offset: 4
## tab-width: 8
## compile-command: "automake --add-missing"
## End:
