#
# Makefile
#
# Version:	$Id: Makefile,v 1.7.2.1.2.1 2006/05/25 16:24:40 nbk Exp $
#

include ../Make.inc

SUBDIRS		= include lib modules main
WHAT_TO_MAKE	= all

all:
	@$(MAKE) $(MFLAGS) WHAT_TO_MAKE=$@ common

clean:
	@$(MAKE) $(MFLAGS) WHAT_TO_MAKE=$@ common
	@rm -f include/*~ *~

install:
	@$(MAKE) $(MFLAGS) WHAT_TO_MAKE=$@ common

common:
	@for dir in $(SUBDIRS); do \
		echo "Making $(WHAT_TO_MAKE) in $$dir..."; \
		$(MAKE) $(MFLAGS) -C $$dir $(WHAT_TO_MAKE) || exit $$?; \
	done
