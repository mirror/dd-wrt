#
# Makefile          libnl Makefile
#
# $Id: Makefile 119 2005-01-21 20:29:58Z tgr $
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

LDFLAGS=-L../lib
CFLAGS+=$(LDFLAGS)

all: addr link neigh tctree route link_direct rule

addr:
	$(CC) $(CFLAGS) -o addr addr.c -lnl

link:
	$(CC) $(CFLAGS) -o link link.c -lnl

link_direct:
	$(CC) $(CFLAGS) -o link_direct link_direct.c -lnl

neigh:
	$(CC) $(CFLAGS) -o neigh neigh.c -lnl

tctree:
	$(CC) $(CFLAGS) -o tctree tctree.c -lnl -lm

route:
	$(CC) $(CFLAGS) -o route route.c -lnl

add_prio:
	$(CC) $(CFLAGS) -o add_prio add_prio.c -lnl

rule:
	$(CC) $(CFLAGS) -o rule rule.c -lnl

clean:
	rm -f test link neigh tctree route link_direct addr add_prio rule

distclean: clean
