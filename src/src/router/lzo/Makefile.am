## Process this file with automake to create Makefile.in
#
# Copyright (C) 1996-2002 Markus F.X.J. Oberhumer
#

AUTOMAKE_OPTIONS = 1.6 gnu

SUBDIRS = include src ltest examples tests minilzo

EXTRA_DIST = BUGS

ACLOCAL_AMFLAGS = -I acconfig/m4


# /***********************************************************************
# // automake targets
# ************************************************************************/

dist-hook:
	-rm -rf ./$(distdir)/acconfig/
	cp -rpi $(srcdir)/B/ $(srcdir)/acconfig/ $(srcdir)/doc/ $(srcdir)/util/ $(distdir)/
	-rm -f `find ./$(distdir)/ -type l -print`
	-rm -f ./$(distdir)/util/*.fil

check-local: all-am
	./ltest/ltest -mlzo -n2 -q $(top_srcdir)/COPYING
	@echo
	@echo "All checks passed."
	@echo


# /***********************************************************************
# // special targets
# ************************************************************************/

basic-test: all
	./ltest/ltest -mlzo -n2 -q $(top_srcdir)/COPYING
	./tests/align
	./tests/chksum
	./examples/simple
	./minilzo/testmini

test: all
	./ltest/ltest -mavail -n10 -q $(top_srcdir)/COPYING
	$(SHELL) -e $(top_srcdir)/util/check.sh $(top_srcdir)
	./tests/align
	./tests/chksum
	./examples/simple
	./minilzo/testmini
	@echo
	@echo "All tests passed. Now you are ready to install LZO."
	@echo

ltest:
	cd src && $(MAKE)
	cd ltest && $(MAKE) ltest

@MAINT@linux-shared:
@MAINT@	CC="$(CC)" $(SHELL) -e $(srcdir)/b/linux/shared.sh


@MAINT@@GMAKE_TRUE@.PHONY: basic-test check-local dist-hook linux-shared ltest test
@MAINT@@GMAKE_TRUE@-include $(top_srcdir)/maint/make/top.mk

