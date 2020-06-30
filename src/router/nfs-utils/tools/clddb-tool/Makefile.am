## Process this file with automake to produce Makefile.in
PYTHON_FILES =  clddb-tool.py

man8_MANS	= clddb-tool.man

EXTRA_DIST	= $(man8_MANS) $(PYTHON_FILES)

all-local: $(PYTHON_FILES)

install-data-hook:
	$(INSTALL) -m 755 clddb-tool.py $(DESTDIR)$(sbindir)/clddb-tool

MAINTAINERCLEANFILES=Makefile.in
