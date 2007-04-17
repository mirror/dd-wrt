#
# Makefile
#
# Version:	$Id: Makefile,v 1.1.2.1 2006/07/15 17:16:51 nbk Exp $
#

include ../../Make.inc

all:

clean:
	@rm -f *~

install:
	$(INSTALL) -d -m 755 $(R)$(docdir)/examples
	for file in *[!~]; do \
		if [ -f $$file -a $$file != Makefile ]; then \
			$(INSTALL) -m 644 $$file $(R)$(docdir)/examples; \
		fi; \
	done

.PHONY: all clean install
