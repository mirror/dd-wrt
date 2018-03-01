# Makefile for Screen documentation

srcdir = @srcdir@
VPATH = @srcdir@

DESTDIR = 

prefix = @prefix@
datarootdir = @datarootdir@
mandir = @mandir@
infodir = @infodir@

INSTALL = @INSTALL@
INSTALL_DATA = @INSTALL_DATA@
MAKEINFO = makeinfo
TEXI2DVI = texi2dvi
TEXI2PDF = texi2pdf

SHELL = /bin/sh

all: screen.info

dvi screen.dvi: screen.texinfo mostlyclean
	$(TEXI2DVI) $(srcdir)/screen.texinfo

pdf screen.pdf: screen.texinfo mostlyclean
	$(TEXI2PDF) $(srcdir)/screen.texinfo

info screen.info: screen.texinfo
	@rm -f screen.info*
	$(MAKEINFO) $(srcdir)/screen.texinfo -o screen.info

install: installdirs
	$(INSTALL_DATA) $(srcdir)/screen.1 $(DESTDIR)$(mandir)/man1/screen.1
	-$(MAKE) screen.info
	-if test -f screen.info; then d=.; else d=$(srcdir); fi; \
	if test -f $$d/screen.info; then \
	for f in $$d/screen.info*; do $(INSTALL_DATA) $$f $(DESTDIR)$(infodir);done; \
	if $(SHELL) -c 'install-info --version' >/dev/null 2>&1; then \
	  install-info --info-dir=$(DESTDIR)$(infodir) $$d/screen.info; \
	else true; fi; \
	fi

uninstall:
	rm -f $(DESTDIR)$(mandir)/man1/screen.1
	rm -f $(DESTDIR)$(infodir)/screen.info*

installdirs:
	$(srcdir)/../etc/mkinstalldirs $(DESTDIR)$(mandir)/man1 $(DESTDIR)$(infodir)

mostlyclean:
	-rm -f *.cp *.cps *.fn *.fns *.ky *.kys *.pg *.tp *.vr
	-rm -f *.log *.aux *.toc *~

clean distclean clobber: mostlyclean
	-rm -f *.dvi

realclean: clean
	-rm -f *.info*

check installcheck:

