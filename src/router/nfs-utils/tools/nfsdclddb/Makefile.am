## Process this file with automake to produce Makefile.in
PYTHON_FILES =  nfsdclddb.py

man8_MANS	= nfsdclddb.man

EXTRA_DIST	= $(man8_MANS) $(PYTHON_FILES)

all-local: $(PYTHON_FILES)

install-data-hook:
	$(INSTALL) -m 755 nfsdclddb.py $(DESTDIR)$(sbindir)/nfsdclddb

MAINTAINERCLEANFILES=Makefile.in
