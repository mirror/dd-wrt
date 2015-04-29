#
# Makefile
#
# Version:	$Id: c02b369be9ba04e6cffcf4ab5eb805d08ec58cc0 $
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
