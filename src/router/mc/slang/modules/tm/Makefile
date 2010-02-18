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
  randfuns.hlp forkfuns.hlp

all: help-files
help-files: $(HLP_FILES)
html: modfuns.html

#----------------------------------------------------------------------------
modfuns.sgml : modfuns.tm $(HLP_FILES)
	$(TM2SGML) modfuns.tm modfuns.sgml
modfuns.html : modfuns.sgml
	$(SGML2HTML) modfuns.sgml
#----------------------------------------------------------------------------
pngfuns.hlp: pngfuns.tm
	$(TMEXPAND) $(MACRODIR) -Mslhlp pngfuns.tm pngfuns.hlp
pcrefuns.hlp: pcrefuns.tm
	$(TMEXPAND) $(MACRODIR) -Mslhlp pcrefuns.tm pcrefuns.hlp
sockfuns.hlp: sockfuns.tm
	$(TMEXPAND) $(MACRODIR) -Mslhlp sockfuns.tm sockfuns.hlp
onigfuns.hlp: onigfuns.tm
	$(TMEXPAND) $(MACRODIR) -Mslhlp onigfuns.tm onigfuns.hlp
randfuns.hlp: randfuns.tm
	$(TMEXPAND) $(MACRODIR) -Mslhlp randfuns.tm randfuns.hlp
forkfuns.hlp: forkfuns.tm
	$(TMEXPAND) $(MACRODIR) -Mslhlp forkfuns.tm forkfuns.hlp
#----------------------------------------------------------------------------
clean:
	-rm -f *~ *.dvi *.log *.aux *.toc *.bak *.sgml
distclean: clean
	-rm -f *.html *.ps $(HLP_FILES)
install: $(HLP_FILES) html
	-mv $(HLP_FILES) ../help
	-mv *.html ../../doc/html/
