## src/vm/jit/intrp/Makefile.am
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


AM_CPPFLAGS += -I$(top_srcdir)/src -I$(top_srcdir)/src/vm/jit/$(ARCH_DIR) -I$(top_srcdir)/src/vm/jit/$(ARCH_DIR)/$(OS_DIR) -I$(top_builddir) -I$(top_builddir)/src

LIBS =

EXTRA_DIST = \
	java.vmg \
	gray.fs \
	prims2x.fs \
	vmgenx

noinst_HEADERS = \
	engine.c

BUILT_SOURCES = \
	java-disasm.i \
	java-gen.i \
	java-labels.i \
	java-names.i \
	java-peephole.i \
	java-profile.i \
	java-superend.i \
	java-vm.i

CLEANFILES = \
	$(BUILT_SOURCES)

noinst_LTLIBRARIES = libintrp.la

libintrp_la_SOURCES = \
	asmpart.c \
	codegen.c \
	codegen.h \
	disasm.c \
	disass.cpp \
	dynamic-super.c \
	engine1.c \
	engine2.c \
	intrp.h \
	md.c \
	patcher.c \
	peephole.c

$(BUILT_SOURCES): $(srcdir)/java.vmg $(srcdir)/prims2x.fs $(srcdir)/vmgenx
	$(SHELL) $(srcdir)/vmgenx $(srcdir)/java.vmg && touch java-peephole.i
	true #GFORTHPATH=.:/nfs/nfstmp/anton/cacao/:/usr/lib/gforth/site-forth:/usr/share/gforth/site-forth:/usr/lib/gforth/0.6.2:/usr/share/gforth/0.6.2 $(srcdir)/vmgenx $(srcdir)/java.vmg && touch java-peephole.i


## Local variables:
## mode: Makefile
## indent-tabs-mode: t
## c-basic-offset: 4
## tab-width: 8
## compile-command: "automake --add-missing"
## End:
