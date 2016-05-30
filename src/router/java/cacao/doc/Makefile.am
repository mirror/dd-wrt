## doc/Makefile.am
##
## Copyright (C) 1996-2005, 2006, 2008
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


SUBDIRS = \
	abc_optimization \
	doxygen \
	handbook \
	intern_tables

dist_noinst_DATA = \
	assertions.tex \
	annotations.tex \
	jsr.bib

EXTRA_DIST = \
	bibliography.bib \
	design_onstack_replacement.txt \
	inlining_stacktrace.txt \
	native_threads.txt \
	stack.txt \
	stack_frames.txt

CLEANFILES = \
	annotations.aux \
	annotations.bbl \
	annotations.blg \
	annotations.dvi \
	annotations.lof \
	annotations.log \
	annotations.lot \
	annotations.toc \
	annotations.idx \
	annotations.out \
	annotations.tex~ \
	assertions.aux \
	assertions.dvi \
	assertions.log \
	assertions.toc

annotations:
	latex annotations
	bibtex annotations
	latex annotations
	latex annotations

assertions:
	latex assertions
	latex assertions

## Local variables:
## mode: Makefile
## indent-tabs-mode: t
## c-basic-offset: 4
## tab-width: 8
## compile-command: "automake --add-missing"
## End:
