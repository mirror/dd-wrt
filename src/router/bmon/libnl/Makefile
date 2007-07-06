#
# Makefile          libnl Makefile
#
# Copyright (c) 2004-2005 Thomas Graf <tgraf@suug.ch>
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
#

ifeq ($(shell [ ! -r Makefile.opts ] && echo 1),)
    include Makefile.opts
endif

SUBDIRS := lib include
.PHONY: all clean distclean install $(SUBDIRS)

all: Makefile.opts
	@for dir in $(SUBDIRS); do \
		echo "Entering $$dir" && cd $$dir && $(MAKE) && cd ..; \
	done

clean: Makefile.opts
	@for dir in $(SUBDIRS); do \
		echo "Entering $$dir" && cd $$dir && $(MAKE) clean && cd ..; \
	done

distclean: clean
	@$(RM) -rf Makefile.opts autom4te.cache config.log config.status
	@for dir in $(SUBDIRS); do \
		echo "Entering $$dir" && cd $$dir && $(MAKE) distclean && cd ..; \
	done

doc:
	doxygen Doxyfile

install: Makefile.opts
	@for dir in $(SUBDIRS); do \
		echo "Entering $$dir" && cd $$dir && $(MAKE) install && cd ..; \
	done

show: Makefile.opts
	@echo "CC:          $(CC)"
	@echo "RM:          $(RM)"
	@echo "CFLAGS:      $(CFLAGS)"
	@echo "DEPFLAGS:    $(DEPFLAGS)"
	@echo "LDFLAGS:     $(LDFLAGS)"


$(SUBDIRS):
	cd $@ && $(MAKE)

-include Makefile.rules
