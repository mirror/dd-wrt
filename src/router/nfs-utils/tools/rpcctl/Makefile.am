## Process this file with automake to produce Makefile.in
PYTHON_FILES =  rpcctl.py

man8_MANS = rpcctl.man

EXTRA_DIST = $(man8_MANS) $(PYTHON_FILES)

all-local: $(PYTHON_FILES)

install-data-hook:
	$(INSTALL) -m 755 rpcctl.py $(DESTDIR)$(sbindir)/rpcctl

MAINTAINERCLEANFILES=Makefile.in
