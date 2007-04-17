#
# Makefile
#
# Version:	$Id: Makefile,v 1.2.4.1 2006/08/16 21:43:18 nbk Exp $
#

include ../../Make.inc
.PHONY: all clean distclean install

all: radpaths.h

radpaths.h: build-radpaths-h
	@/bin/sh ./build-radpaths-h

distclean:
	rm -f radpaths.h

clean:

install:
