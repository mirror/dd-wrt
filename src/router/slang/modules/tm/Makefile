# -*- sh -*-
#
# To create the SGML files, you will need to install the tm-utils
# package.  See http://www.jedsoft.org/ for more information.
#
#TMEXPAND = $(HOME)/lib/tmexpand/bin/tmexpand
TMEXPAND=tmexpand
MACRODIR = # -I$(HOME)/lib/tmexpand/macros
TM2SGML = tmexpand
SGML2HTML = sgml2html

HLP_FILES = pngfuns.hlp pcrefuns.hlp sockfuns.hlp onigfuns.hlp \
  randfuns.hlp forkfuns.hlp csvfuns.hlp slsmg.hlp

all: help-files
help-files: $(HLP_FILES)
html: modfuns.html

.SUFFIXES: .tm .hlp
.tm.hlp:
	$(TMEXPAND) $(MACRODIR) -Mslhlp $< $@
#----------------------------------------------------------------------------
modfuns.sgml : modfuns.tm $(HLP_FILES)
	$(TM2SGML) modfuns.tm modfuns.sgml
modfuns.html : modfuns.sgml
	$(SGML2HTML) modfuns.sgml
#----------------------------------------------------------------------------
clean:
	-rm -f *~ *.dvi *.log *.aux *.toc *.bak *.sgml
distclean: clean
	-rm -f *.html *.ps $(HLP_FILES)
install: install-help install-html
install-help: $(HLP_FILES)
	-mv $(HLP_FILES) ../help
install-html: html
	-mv *.html ../../doc/html/
.PHONY: iclean distclean install install-help install-html
