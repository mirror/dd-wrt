# -*- sh -*-
#
# To create the SGML files, you will need to install the tmexpand
# package.  See http://www.jedsoft.org/ for more information.
#
#TMEXPAND = $(HOME)/lib/tmexpand/bin/tmexpand
TMEXPAND = tmexpand
MACRODIR = -I$(HOME)/lib/tmexpand/macros
TM2SGML = $(TMEXPAND)
TM2HLP = $(TMEXPAND) -Mslhlp

TXT_FILES = slang.txt cslang.txt cref.txt slangfun.txt
SGML_FILES = slang.sgml cslang.sgml cref.sgml slangfun.sgml
HTML_FILES = slang.html cslang.html cref.html slangfun.html
TEX_FILES = slang.tex cslang.tex cref.tex #slangfun.tex
PS_FILES = slang.ps cslang.ps slangfun.ps #cref.ps
PDF_FILES = slang.pdf cslang.pdf slangfun.pdf cref.pdf

SGML2LATEX = sgml2latex -p letter -o tex
SGML2HTML = sgml2html
#SGML2TXT = sgml2txt -f
SGML2TXT = linuxdoc -B txt -f
TM2TXT = tools/$(ARCH)objs/tm2txt --quiet
LATEX = latex
PDFLATEX = pdflatex

TEXTDIR = ../text
PSDIR = ../ps
HTMLDIR = ../html
PDFDIR = ../pdf

SUBDIRS = $(TEXTDIR) $(HTMLDIR) $(PSDIR) $(SGMLDIR) $(PDFDIR)
SRCDIR = `pwd`

text-files: $(TXT_FILES)
all: $(SGML_FILES) $(HTML_FILES) $(TEX_FILES) $(TXT_FILES) $(PDF_FILES) $(PS_FILES)
#----- SGML Files -----------------------------------------------------------
cslang.sgml : cslang.tm preface.tm copyright.tm local.tm apinews.tm
	$(TM2SGML) cslang.tm cslang.sgml
slang.sgml : slang.tm preface.tm copyright.tm local.tm intnews.tm
	$(TM2SGML) slang.tm slang.sgml
cref.sgml : cref.tm crtl/*.tm
	$(TM2SGML) cref.tm cref.sgml
slangfun.sgml : slangfun.tm local.tm rtl/*.tm
	$(TM2SGML) slangfun.tm slangfun.sgml
#----- HTML Files -----------------------------------------------------------
html-files: $(HTML_FILES)
cslang.html : cslang.sgml
	$(SGML2HTML) cslang.sgml
slang.html : slang.sgml
	$(SGML2HTML) slang.sgml
cref.html : cref.sgml
	$(SGML2HTML) cref.sgml
slangfun.html : slangfun.sgml
	$(SGML2HTML) slangfun.sgml
#----- TeX Files ------------------------------------------------------------
slang.tex : slang.sgml
	$(SGML2LATEX) slang.sgml
	jed -script ./fixtex.sl slang.tex
cslang.tex : cslang.sgml
	$(SGML2LATEX) cslang.sgml
	jed -script ./fixtex.sl cslang.tex
slangfun.tex: slangfun.sgml
	$(SGML2LATEX) slangfun.sgml
	jed -script ./fixtex.sl slangfun.tex
cref.tex: cref.sgml
	$(SGML2LATEX) cref.sgml
	jed -script ./fixtex.sl cref.tex
#----- PDF Files -----------------------------------------------------------
pdf-files: $(PDF_FILES)
cslang.pdf : cslang.tex
	$(PDFLATEX) cslang.tex
	$(PDFLATEX) cslang.tex
	$(PDFLATEX) cslang.tex
slang.pdf : slang.tex
	$(PDFLATEX) slang.tex
	$(PDFLATEX) slang.tex
	$(PDFLATEX) slang.tex
slangfun.pdf: slangfun.tex
	$(PDFLATEX) slangfun.tex
	$(PDFLATEX) slangfun.tex
	$(PDFLATEX) slangfun.tex
cref.pdf : cref.tex
	$(PDFLATEX) cref.tex
	$(PDFLATEX) cref.tex
	$(PDFLATEX) cref.tex
#----- PS Files -----------------------------------------------------------
cslang.ps : cslang.tex
	$(LATEX) cslang.tex
	$(LATEX) cslang.tex
	dvips -o cslang.ps cslang.dvi
slang.ps : slang.tex
	$(LATEX) slang.tex
	$(LATEX) slang.tex
	dvips -o slang.ps slang.dvi
slangfun.ps : slangfun.tex
	$(LATEX) slangfun.tex
	$(LATEX) slangfun.tex
	dvips -o slangfun.ps slangfun.dvi
#----- Text Files -----------------------------------------------------------
cslang.txt: cslang.sgml
	$(SGML2TXT) cslang.sgml
	./tools/fixtxt cslang.txt
slang.txt: slang.sgml
	$(SGML2TXT) slang.sgml
	./tools/fixtxt slang.txt
#
slangfun.txt : slangfun.tm hlpdefs.tm rtl/*.tm
	cat hlpdefs.tm rtl/*.tm | $(TM2HLP) - slangfun.txt
#	cat rtl/*.tm | $(TM2TXT) > slangfun.txt
cref.txt : cref.sgml
	$(SGML2TXT) cref.sgml
	./tools/fixtxt cref.txt
#	$(TM2TXT) < cref.tm > cref.txt
$(TM2TXT) : tools/tm2txt.c
	cd tools; make SRCDIR=$(SRCDIR)
#----------------------------------------------------------------------------
clean:
	-rm -f *~ *.dvi *.log *.aux *.toc rtl/*.BAK rtl/*~ crtl/*~ *.tmp *-error
distclean: clean
	-rm -f *.html *.ps $(TXT_FILES) $(TEX_FILES) $(SGML_FILES) $(PDF_FILES)
	cd tools; make clean
install: $(TXT_FILES)
	-mv $(TXT_FILES) ../text

install-all: all
	-mkdir $(SUBDIRS)
	-mv $(TXT_FILES) ../text
	-mv *.html ../html
	-mv $(PS_FILES) ../ps
	-mv $(PDF_FILES) ../pdf
