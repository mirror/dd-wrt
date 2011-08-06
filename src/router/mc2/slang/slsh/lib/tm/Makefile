# -*- sh -*-
#
# To create the SGML files, you will need to install the tm-utils
# package.  See http://www.jedsoft.org/ for more information.
#
TMEXPAND = $(HOME)/lib/tmexpand/bin/tmexpand
MACRODIR = $(HOME)/lib/tmexpand/macros

HLP_FILES = arrayfuns.hlp glob.hlp require.hlp structfuns.hlp cmdopt.hlp \
            readascii.hlp profile.hlp print.hlp process.hlp

all: help-files
help-files: $(HLP_FILES)
arrayfuns.hlp: arrayfuns.tm
	$(TMEXPAND) -I$(MACRODIR) -Mslhlp arrayfuns.tm arrayfuns.hlp
glob.hlp: glob.tm
	$(TMEXPAND) -I$(MACRODIR) -Mslhlp glob.tm glob.hlp
require.hlp: require.tm
	$(TMEXPAND) -I$(MACRODIR) -Mslhlp require.tm require.hlp
structfuns.hlp: structfuns.tm
	$(TMEXPAND) -I$(MACRODIR) -Mslhlp structfuns.tm structfuns.hlp
cmdopt.hlp: cmdopt.tm
	$(TMEXPAND) -I$(MACRODIR) -Mslhlp cmdopt.tm cmdopt.hlp
print.hlp: print.tm
	$(TMEXPAND) -I$(MACRODIR) -Mslhlp print.tm print.hlp
readascii.hlp: readascii.tm
	$(TMEXPAND) -I$(MACRODIR) -Mslhlp readascii.tm readascii.hlp
profile.hlp: profile.tm
	$(TMEXPAND) -I$(MACRODIR) -Mslhlp profile.tm profile.hlp
process.hlp: process.tm
	$(TMEXPAND) -I$(MACRODIR) -Mslhlp process.tm process.hlp
#----------------------------------------------------------------------------
clean:
	-rm -f *~ *.dvi *.log *.aux *.toc *.bak
distclean: clean
	-rm -f *.html *.ps $(HLP_FILES)
install: $(HLP_FILES)
	-mv $(HLP_FILES) ../help
