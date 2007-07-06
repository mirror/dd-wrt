# Makefile                     bmon Makefile
#
# $Id: GNUmakefile 16 2004-10-29 12:04:07Z tgr $
#
# Copyright (c) 2001-2004 Thomas Graf <tgraf@suug.ch>
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.

ifeq ($(shell [ ! -r Makefile.opts ] && echo 1),)
    include Makefile.opts
endif

SUBDIRS := src
.PHONY: all clean distclean install $(SUBDIRS)

export

all: Makefile.opts
	@for dir in $(SUBDIRS); do \
		echo "Entering $$dir" && cd $$dir && $(MAKE) && cd ..; \
	done

clean: 
	@for dir in $(SUBDIRS); do \
		echo "Entering $$dir" && cd $$dir && $(MAKE) clean && cd ..; \
	done

distclean: clean
	@rm -rf autom4te.cache config.log config.status Makefile.opts include/bmon/defs.h
	@for dir in $(SUBDIRS); do \
		echo "Entering $$dir" && cd $$dir && $(MAKE) distclean && cd ..; \
	done

install:
	./install-sh -d -m 0755 $(DESTDIR)$(prefix)/bin
	./install-sh -d -m 0755 $(DESTDIR)$(mandir)/man1
	./install-sh -c -s -m 0755 src/bmon $(DESTDIR)$(prefix)/bin
	./install-sh -c    -m 0644 man/bmon.1 $(DESTDIR)$(mandir)/man1

show: Makefile.opts
	@echo "CC:          $(CC)"
	@echo "RM:          $(RM)"
	@echo "CFLAGS:      $(CFLAGS)"
	@echo "CPPFLAGS:    $(CPPFLAGS)"
	@echo "DEPFLAGS:    $(DEPFLAGS)"
	@echo "LDFLAGS:     $(LDFLAGS)"

-include Makefile.rules
