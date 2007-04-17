#
# Makefile
#
# Version:	$Id: Makefile,v 1.4.4.2.2.4 2007/04/04 09:23:57 nbk Exp $
#

include ../Make.inc

SUBDIRS		= examples rfc
WHAT_TO_MAKE	= all

all:
	@$(MAKE) $(MFLAGS) WHAT_TO_MAKE=$@ common

clean:
	@$(MAKE) $(MFLAGS) WHAT_TO_MAKE=$@ common
	@rm -f *~

install:
	@if [ "$(docdir)" = no ]; then					\
		echo 'Documentation files will NOT be installed.';	\
	else								\
		$(INSTALL) -d -m 755 $(R)$(docdir);			\
		for file in *[!~]; do					\
			if [ -f $$file -a $$file != Makefile ]; then	\
				$(INSTALL) -m 644 $$file $(R)$(docdir);	\
			fi;						\
		done;							\
		$(MAKE) $(MFLAGS) WHAT_TO_MAKE=$@ common;		\
	fi

common: $(SUBDIRS)

$(SUBDIRS):
	@echo "Making $(WHAT_TO_MAKE) in $@..."
	@$(MAKE) $(MFLAGS) -C $@ $(WHAT_TO_MAKE)

.PHONY: all clean install common $(SUBDIRS)
