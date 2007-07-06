#
# Makefile          libnl Makefile
#
# $Id: Makefile 43 2004-11-15 18:04:53Z tgr $
#
# Copyright (c) 2004 Thomas Graf <tgraf@suug.ch>
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
	mkdir -p $(DESTDIR)$(prefix)/include/netlink/route/
	install -o root -g root -m 0644 netlink/*.h $(DESTDIR)$(prefix)/include/netlink/
	install -o root -g root -m 0644 netlink/route/*.h $(DESTDIR)$(prefix)/include/netlink/route/
