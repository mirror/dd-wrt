## $Id: Makefile.am 601 2005-02-25 22:44:39Z dugsong $

include $(top_srcdir)/Makefile.am.common

if PYTHON
# XXX - nasty automake fudging here...

noinst_SCRIPTS = python-build

$(srcdir)/dnet.c: $(srcdir)/dnet.pyx
	pyrexc $(srcdir)/dnet.pyx

python-build: $(srcdir)/dnet.c
	$(PYTHON) setup.py build
	touch python-build

python-clean:
	rm -rf *build

clean: python-clean

distclean: clean

python-install:
	$(PYTHON) setup.py install --prefix=$(DESTDIR)$(prefix)

install-exec-local: python-install

TESTS = test.py

else

install-exec-local:

endif

EXTRA_DIST = dnet.pyx dnet.c setup.py.in test.py

DISTCLEANFILES = *~
