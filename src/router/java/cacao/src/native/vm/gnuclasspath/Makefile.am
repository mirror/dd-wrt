## src/native/vm/gnuclasspath/Makefile.am
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

if ENABLE_JVMTI
lib_LTLIBRARIES = \
	libjdwp.la
endif

if ENABLE_ANNOTATIONS
SUN_REFLECT_SOURCES = \
	sun_reflect_ConstantPool.cpp
endif

libnativevmcore_la_SOURCES = \
	gnu_classpath_VMStackWalker.cpp \
	gnu_classpath_VMSystemProperties.cpp \
	gnu_java_lang_VMCPStringBuilder.cpp \
	gnu_java_lang_management_VMClassLoadingMXBeanImpl.cpp \
	gnu_java_lang_management_VMMemoryMXBeanImpl.cpp \
	gnu_java_lang_management_VMRuntimeMXBeanImpl.cpp \
	gnu_java_lang_management_VMThreadMXBeanImpl.cpp \
	java_lang_VMClass.cpp \
	java_lang_VMClassLoader.cpp \
	java_lang_VMObject.cpp \
	java_lang_VMRuntime.cpp \
	java_lang_VMString.cpp \
	java_lang_VMSystem.cpp \
	java_lang_VMThread.cpp \
	java_lang_VMThrowable.cpp \
	java_lang_management_VMManagementFactory.cpp \
	java_lang_reflect_VMConstructor.cpp \
	java_lang_reflect_VMField.cpp \
	java_lang_reflect_VMMethod.cpp \
	java_lang_reflect_VMProxy.cpp \
	java_security_VMAccessController.cpp \
	java_util_concurrent_atomic_AtomicLong.cpp \
	$(SUN_REFLECT_SOURCES)

if ENABLE_JVMTI
libjdwp_la_SOURCES = \
	gnu_classpath_jdwp_VMFrame.cpp \
	gnu_classpath_jdwp_VMMethod.cpp \
	gnu_classpath_jdwp_VMVirtualMachine.cpp \
	VMjdwp.cpp \
	VMjdwp.hpp
endif


## Local variables:
## mode: Makefile
## indent-tabs-mode: t
## c-basic-offset: 4
## tab-width: 8
## compile-command: "automake --add-missing"
## End:
