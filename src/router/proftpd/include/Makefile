

top_builddir=..
top_srcdir=..
srcdir=.

LS=/bin/ls


include ../Make.rules

.SUFFIXES: .h

.PHONY: ""

all:
	@echo "Running make from top level directory."
	cd .. && $(MAKE) all

Makefile: Makefile.in ../config.status
	cd ../ && ./config.status

install:
	@headers=`$(LS) *.h`; \
	for h in $$headers; do \
		$(INSTALL_MAN) $$h $(DESTDIR)$(includedir)/proftpd/$$h; \
	done
