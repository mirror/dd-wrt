# -*- sh -*-
#
# To create the SGML files, you will need to install the tm-utils
# package.  See http://www.jedsoft.org/ for more information.
#
TMEXPAND = tmexpand

HLP_FILES = arrayfuns.hlp glob.hlp require.hlp structfuns.hlp cmdopt.hlp \
            readascii.hlp profile.hlp print.hlp process.hlp setfuns.hlp

all: help-files
help-files: $(HLP_FILES)
.SUFFIXES: .tm .hlp
.tm.hlp:
	$(TMEXPAND) -Mslhlp $< $@
clean:
	-rm -f *~ *.dvi *.log *.aux *.toc *.bak
distclean: clean
	-rm -f *.html *.ps $(HLP_FILES)
install: $(HLP_FILES)
	-mv $(HLP_FILES) ../help
