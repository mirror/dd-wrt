## src/vm/jit/Makefile.am
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
	allocator \
	inline \
	ir \
	loop \
	optimizing \
	schedule \
	verify \
	\
	intrp \
	alpha \
	arm \
	i386 \
	mips \
	parisc \
	powerpc \
	powerpc64 \
	s390 \
	sparc64 \
	x86_64

SUBDIRS = \
	ir \
	optimizing \
	$(ARCH_DIR)

ARCH_LIB = \
	$(ARCH_DIR)/libarch.la

if ENABLE_JIT
SUBDIRS += \
	allocator

OPTIMIZING_LIB = \
	optimizing/liboptimizing.la
endif

if ENABLE_INTRP
SUBDIRS += \
	intrp

INTRP_LIB = \
	intrp/libintrp.la
endif

if ENABLE_INLINING
SUBDIRS += \
	inline

INLINE_LIB = \
	inline/libinline.la
endif

if ENABLE_LOOP
SUBDIRS += \
	loop

LOOP_LIB = \
	loop/libloop.la
endif

if USE_SCHEDULER
SUBDIRS += \
	schedule

SCHEDULE_LIB = \
	schedule/libschedule.la
endif

REPLACE_SOURCES =

if ENABLE_JIT
REG_SOURCES = \
	reg.cpp \
	reg.hpp

REPLACE_SOURCES += \
	replace.hpp

STACK_SOURCES = \
	stack.cpp \
	stack.hpp

TRAP_SOURCES = \
	trap.cpp \
	trap.hpp
endif

if ENABLE_REPLACEMENT
REPLACE_SOURCES += \
	replace.cpp
endif

if ENABLE_VERIFIER
SUBDIRS += \
	verify

VERIFIER_LIB = \
	verify/libverify.la
endif

if ENABLE_OPAGENT
OPAGENT_SOURCES = \
	oprofile-agent.cpp \
	oprofile-agent.hpp

endif

if WITH_BINUTILS_DISASSEMBLER
DISASS_SOURCES = disass-common.cpp
endif

noinst_HEADERS = \
	abi.hpp \
	abi-asm.hpp \
	asmpart.hpp \
	methodheader.hpp

noinst_LTLIBRARIES = \
	libjit.la

libjit_la_SOURCES = \
	argument.cpp \
	argument.hpp \
	builtin.cpp \
	builtin.hpp \
	builtintable.inc \
	code.cpp \
	code.hpp \
	codegen-common.cpp \
	codegen-common.hpp \
	disass.hpp \
	$(DISASS_SOURCES) \
	dseg.cpp \
	dseg.hpp \
	emit-common.cpp \
	emit-common.hpp \
	exceptiontable.cpp \
	exceptiontable.hpp \
	executionstate.cpp \
	executionstate.hpp \
	jit.cpp \
	jit.hpp \
	linenumbertable.cpp \
	linenumbertable.hpp \
	methodtree.cpp \
	methodtree.hpp \
	parse.cpp \
	parse.hpp \
	patcher-common.cpp \
	patcher-common.hpp \
	$(RECOMPILE_SOURCES) \
	$(REG_SOURCES) \
	$(REPLACE_SOURCES) \
	show.cpp \
	show.hpp \
	$(STACK_SOURCES) \
	stacktrace.cpp \
	stacktrace.hpp \
	stubs.cpp \
	stubs.hpp \
	trace.cpp \
	trace.hpp \
	$(TRAP_SOURCES) \
	$(OPAGENT_SOURCES)

libjit_la_SOURCES += \
	cfg.cpp \
	cfg.hpp

libjit_la_LIBADD = \
	allocator/liballocator.la \
	$(INLINE_LIB) \
	ir/libir.la \
	$(LOOP_LIB) \
	$(OPTIMIZING_LIB) \
	$(PROFILE_LIB) \
	$(SCHEDULE_LIB) \
	$(VERIFIER_LIB) \
	$(INTRP_LIB) \
	$(ARCH_LIB)

## Local variables:
## mode: Makefile
## indent-tabs-mode: t
## c-basic-offset: 4
## tab-width: 8
## compile-command: "automake --add-missing"
## End:
