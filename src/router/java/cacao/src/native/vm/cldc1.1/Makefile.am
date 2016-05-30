## src/native/vm/cldc1.1/Makefile.am
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

noinst_LTLIBRARIES = \
	libnativevmcore.la

libnativevmcore_la_SOURCES = \
	com_sun_cldc_io_ResourceInputStream.cpp \
	com_sun_cldc_io_j2me_socket_Protocol.cpp \
	com_sun_cldchi_io_ConsoleOutputStream.cpp \
	com_sun_cldchi_jvm_JVM.cpp \
	java_lang_Class.cpp \
	java_lang_Double.cpp \
	java_lang_Float.cpp \
	java_lang_Math.cpp \
	java_lang_Object.cpp \
	java_lang_Runtime.cpp \
	java_lang_String.cpp \
	java_lang_System.cpp \
	java_lang_Thread.cpp \
	java_lang_Throwable.cpp


## Local variables:
## mode: Makefile
## indent-tabs-mode: t
## c-basic-offset: 4
## tab-width: 8
## compile-command: "automake --add-missing"
## End:
