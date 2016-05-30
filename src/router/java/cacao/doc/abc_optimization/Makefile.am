## doc/abc_optimization/Makefile.am
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

LATEX = TEXINPUTS=.:$(abs_srcdir):$$TEXINPUTS latex -interaction=batchmode -halt-on-error
PDFLATEX = TEXINPUTS=.:$(abs_srcdir):$$TEXINPUTS pdflatex -interaction=batchmode -halt-on-error
BIBTEX = BIBINPUTS=.:$(abs_srcdir) bibtex

dist_noinst_DATA = \
	BakkalaureatSeminararbeitKB2010.tex \
	bibfile.bib

CLEANFILES = \
	*.aux *.bbl *.blg *.dvi *.lof *.log *.lot *.toc

#pdf: BakkalaureatSeminararbeitKB2010.pdf

#dvi: BakkalaureatSeminararbeitKB2010.dvi

BakkalaureatSeminararbeitKB2010.pdf: $(dist_noinst_DATA)
	$(PDFLATEX) BakkalaureatSeminararbeitKB2010.tex
	$(BIBTEX) BakkalaureatSeminararbeitKB2010.aux
	$(PDFLATEX) BakkalaureatSeminararbeitKB2010.tex
	$(PDFLATEX) BakkalaureatSeminararbeitKB2010.tex

BakkalaureatSeminararbeitKB2010.dvi: $(dist_noinst_DATA)
	$(LATEX) BakkalaureatSeminararbeitKB2010.tex
	$(BIBTEX) BakkalaureatSeminararbeitKB2010.aux
	$(LATEX) BakkalaureatSeminararbeitKB2010.tex
	$(LATEX) BakkalaureatSeminararbeitKB2010.tex

## Local variables:
## mode: Makefile
## indent-tabs-mode: t
## c-basic-offset: 4
## tab-width: 8
## compile-command: "automake --add-missing"
## End:
