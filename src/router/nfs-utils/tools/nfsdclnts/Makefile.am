## Process this file with automake to produce Makefile.in
PYTHON_FILES = nfsdclnts.py

man8_MANS       = nfsdclnts.man

EXTRA_DIST      = $(man8_MANS) $(PYTHON_FILES)

all-local: $(PYTHON_FILES)

install-data-hook:
	$(INSTALL) -m 755 nfsdclnts.py $(DESTDIR)$(sbindir)/nfsdclnts

MAINTAINERCLEANFILES=Makefile.in
