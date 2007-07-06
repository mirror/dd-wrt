#
# include/Makefile
#
# 	This library is free software; you can redistribute it and/or
#	modify it under the terms of the GNU Lesser General Public
#	License as published by the Free Software Foundation version 2.1
#	of the License.
#
# Copyright (c) 2003-2006 Thomas Graf <tgraf@suug.ch>
#

ifeq ($(shell [ ! -r ../Makefile.opts ] && echo 1),)
	include ../Makefile.opts
endif

.PHONY: all clean install 

all:
	@true

clean:
	@true

distclean:
	@true

install:
	mkdir -p $(DESTDIR)$(includedir)/netlink/route/sch/
	mkdir -p $(DESTDIR)$(includedir)/netlink/route/cls/
	mkdir -p $(DESTDIR)$(includedir)/netlink/fib_lookup/
	install -m 0644 netlink/*.h $(DESTDIR)$(includedir)/netlink/
	install -m 0644 netlink/route/*.h $(DESTDIR)$(includedir)/netlink/route/
	install -m 0644 netlink/route/sch/*.h $(DESTDIR)$(includedir)/netlink/route/sch/
	install -m 0644 netlink/route/cls/*.h $(DESTDIR)$(includedir)/netlink/route/cls/
	install -m 0644 netlink/fib_lookup/*.h $(DESTDIR)$(includedir)/netlink/fib_lookup/

