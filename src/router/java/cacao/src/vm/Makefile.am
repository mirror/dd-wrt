## src/vm/Makefile.am
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


AM_CPPFLAGS += -I$(top_srcdir)/src -I$(top_srcdir)/src/vm/jit/$(ARCH_DIR) -I$(top_srcdir)/src/vm/jit/$(ARCH_DIR)/$(OS_DIR) -I$(top_builddir)/src -I$(top_srcdir)/src/native

LIBS =

SUBDIRS = jit

if ENABLE_ASSERTION
ASSERTION_SOURCES = \
	assertion.cpp \
	assertion.hpp
endif

if ENABLE_CYCLES_STATS
CYCLES_STATS_SOURCES = \
	cycles-stats.cpp \
	cycles-stats.hpp
endif

if ENABLE_JAVASE
if ENABLE_ANNOTATIONS
ANNOTATION_SOURCES = \
	annotation.cpp \
	annotation.hpp
endif

STACKMAP_SOURCES = \
	stackmap.cpp \
	stackmap.hpp
endif

if ENABLE_RT_TIMING
RT_TIMING_SOURCES = \
	rt-timing.cpp \
	rt-timing.hpp
endif

if ENABLE_STATISTICS
STATISTICS_SOURCES = \
	statistics.cpp \
	statistics.hpp
endif

if ENABLE_ZLIB
ZLIB_SOURCES = \
	zip.cpp \
	zip.hpp
endif

noinst_HEADERS = \
	global.hpp \
	types.hpp

noinst_LTLIBRARIES = \
	libvm.la

libvm_la_SOURCES = \
	access.cpp \
	access.hpp \
	$(ANNOTATION_SOURCES) \
	array.cpp \
	array.hpp \
	$(ASSERTION_SOURCES) \
	breakpoint.hpp \
	class.cpp \
	class.hpp \
	classcache.cpp \
	classcache.hpp \
	$(CYCLES_STATS_SOURCES) \
	descriptor.cpp \
	descriptor.hpp \
	exceptions.cpp \
	exceptions.hpp \
	field.cpp \
	field.hpp \
	finalizer.cpp \
	finalizer.hpp \
	globals.cpp \
	globals.hpp \
	hook.cpp \
	hook.hpp \
	initialize.cpp \
	initialize.hpp \
	javaobjects.cpp \
	javaobjects.hpp \
	linker.cpp \
	linker.hpp \
	loader.cpp \
	loader.hpp \
	method.cpp \
	method.hpp \
	options.cpp \
	options.hpp \
	os.cpp \
	os.hpp \
	package.cpp \
	package.hpp \
	primitive.cpp \
	primitive.hpp \
	properties.cpp \
	properties.hpp \
	references.hpp \
	resolve.cpp \
	resolve.hpp \
	$(RT_TIMING_SOURCES) \
	signal.cpp \
	signallocal.hpp \
	$(STACKMAP_SOURCES) \
	$(STATISTICS_SOURCES) \
	string.cpp \
	string.hpp \
	suck.cpp \
	suck.hpp \
	utf8.cpp \
	utf8.hpp \
	utf8.inc \
	vftbl.hpp \
	vm.cpp \
	vm.hpp \
	$(ZLIB_SOURCES)

libvm_la_LIBADD = \
	jit/libjit.la


## Local variables:
## mode: Makefile
## indent-tabs-mode: t
## c-basic-offset: 4
## tab-width: 8
## compile-command: "automake --add-missing"
## End:
