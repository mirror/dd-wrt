## src/threads/Makefile.am
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

LIB =

DIST_SUBDIRS = \
	none \
	posix

if ENABLE_THREADS
SUBDIRS = \
	posix

THREAD_LIB = \
	posix/libthreadsposix.la
else
SUBDIRS = \
	none

THREAD_LIB = \
	none/libthreadsnone.la
endif

if WITH_JAVA_RUNTIME_LIBRARY_GNU_CLASSPATH
THREAD_RUNTIME_SOURCES = ThreadRuntime-classpath.cpp
else
if WITH_JAVA_RUNTIME_LIBRARY_CLDC1_1
THREAD_RUNTIME_SOURCES = ThreadRuntime-cldc11.cpp
else
if WITH_JAVA_RUNTIME_LIBRARY_OPENJDK
THREAD_RUNTIME_SOURCES = ThreadRuntime-openjdk.cpp
endif
endif
endif

noinst_LTLIBRARIES = \
	libthreads.la

libthreads_la_SOURCES = \
	atomic.cpp \
	atomic.hpp \
	condition.hpp \
	lock.cpp                \
	lock.hpp \
	lockword.cpp \
	lockword.hpp \
	removeme.cpp \
	mutex.hpp \
	threadlist.cpp \
	threadlist.hpp \
	ThreadRuntime.hpp \
	$(THREAD_RUNTIME_SOURCES) \
	thread.cpp \
	thread.hpp

libthreads_la_LIBADD = \
	$(THREAD_LIB)


## Local variables:
## mode: Makefile
## indent-tabs-mode: t
## c-basic-offset: 4
## tab-width: 8
## compile-command: "automake --add-missing"
## End:
