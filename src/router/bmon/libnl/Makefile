#
# Makefile
#
# 	This library is free software; you can redistribute it and/or
#	modify it under the terms of the GNU Lesser General Public
#	License as published by the Free Software Foundation version 2.1
#	of the License.
#
# Copyright (c) 2003-2006 Thomas Graf <tgraf@suug.ch>
#

ifeq ($(shell [ ! -r Makefile.opts ] && echo 1),)
    include Makefile.opts
endif

SUBDIRS := lib include doc src
.PHONY: all clean distclean install gendoc $(SUBDIRS)

all: Makefile.opts
	@for dir in $(SUBDIRS); do \
		echo "Entering $$dir" && cd $$dir && $(MAKE) && cd ..; \
	done

clean: Makefile.opts
	rm -f cscope.*
	@for dir in $(SUBDIRS); do \
		echo "Entering $$dir" && cd $$dir && $(MAKE) clean && cd ..; \
	done

distclean: clean
	@$(RM) -rf Makefile.opts autom4te.cache config.log config.status
	@for dir in $(SUBDIRS); do \
		echo "Entering $$dir" && cd $$dir && $(MAKE) distclean && cd ..; \
	done

install: Makefile.opts
	@for dir in $(SUBDIRS); do \
		echo "Entering $$dir" && cd $$dir && $(MAKE) install && cd ..; \
	done
#	mkdir -p $(DESTDIR)$(libdir)/pkgconfig/
#	install -m 0644 libnl-1.pc $(DESTDIR)$(libdir)/pkgconfig/

gendoc:
	$(MAKE) -C doc gendoc

show: Makefile.opts
	@echo "CC:          $(CC)"
	@echo "RM:          $(RM)"
	@echo "CFLAGS:      $(CFLAGS)"
	@echo "DEPFLAGS:    $(DEPFLAGS)"
	@echo "LDFLAGS:     $(LDFLAGS)"
	@echo "DESTDIR:     $(DESTDIR)"
	@echo "prefix:      $(prefix)"
	@echo "libdir:      $(libdir)"
	@echo "includedir:  $(includedir)"

cscope:
	cscope -b -q -R -Iinclude -slib -ssrc


$(SUBDIRS):
	cd $@ && $(MAKE)

-include Makefile.rules
