# -*- sh -*-
#
# To create the SGML files, you will need to install the tmexpand
# package.  See http://www.jedsoft.org/ for more information.
#
TM2SGML = tmexpand
SGML2HTML = sgml2html
DOCBOOK2MAN = docbook2man

all: slsh.1
html: slshfun.html

slsh.1: slsh.sgml
	$(DOCBOOK2MAN) slsh.sgml
slsh.sgml: slsh.tm
	$(TM2SGML) slsh.tm slsh.sgml
view: slsh.1
	nroff -man slsh.1 | most
#----------------------------------------------------------------------------
slshfun.sgml : slshfun.tm
	$(TM2SGML) slshfun.tm slshfun.sgml
slshfun.html : slshfun.sgml
	$(SGML2HTML) slshfun.sgml
#----------------------------------------------------------------------------
clean:
	-rm -f *~ *-error manpage.refs manpage.links
distclean: clean
	-rm -f slsh.1 slsh.sgml
install: slsh.1 slshfun.html
	-mkdir ../man
	-mv slsh.1 ../man
	-mkdir ../html
	-mv slshfun*.html ../html
