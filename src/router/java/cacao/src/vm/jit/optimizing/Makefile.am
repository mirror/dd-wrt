## src/vm/jit/optimizing/Makefile.am
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
## Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
## 02111-1307, USA.


AM_CPPFLAGS += -I$(top_srcdir)/src -I$(top_srcdir)/src/vm/jit -I$(top_srcdir)/src/vm/jit/$(ARCH_DIR) -I$(top_srcdir)/src/vm/jit/$(ARCH_DIR)/$(OS_DIR) -I$(top_builddir)/src

LIBS =

if ENABLE_IFCONV
IFCONV_SOURCES = \
	ifconv.cpp \
	ifconv.hpp
endif

if ENABLE_PROFILING
PROFILE_SOURCES = \
	profile.cpp \
	profile.hpp

REORDER_SOURCES = \
	reorder.cpp \
	reorder.hpp
endif

if ENABLE_THREADS
RECOMPILER_SOURCES = \
	recompiler.cpp \
	recompiler.hpp
endif

if ENABLE_SSA
SSA_SOURCES = \
	lsra.cpp \
	lsra.hpp \
	ssa.cpp \
	ssa.hpp \
	ssa_phi.cpp \
	ssa_phi.hpp \
	ssa_rename.cpp \
	ssa_rename.hpp \
	graph.cpp \
	graph.hpp \
	dominators.cpp \
	dominators.hpp \
	lifetimes.cpp \
	lifetimes.hpp \
	ssa2.cpp \
	ssa3.cpp 
endif

if ENABLE_ESCAPE
ESCAPE_SOURCES = \
	bytecode_escape.cpp \
	escape.hpp \
	escape.cpp
endif

noinst_LTLIBRARIES = \
	liboptimizing.la

liboptimizing_la_SOURCES = \
	$(IFCONV_SOURCES) \
	$(PROFILE_SOURCES) \
	$(RECOMPILER_SOURCES) \
	$(REORDER_SOURCES) \
	$(SSA_SOURCES) \
	$(ESCAPE_SOURCES)


## Local variables:
## mode: Makefile
## indent-tabs-mode: t
## c-basic-offset: 4
## tab-width: 8
## compile-command: "automake --add-missing"
## End:
