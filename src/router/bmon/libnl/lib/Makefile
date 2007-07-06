#
# Makefile            rtnetlink
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

# Core
CIN      := $(wildcard *.c)
# NETLINK_ROUTE
CIN      += $(wildcard route/*.c)
# Schedulers
CIN      += route/sch/fifo.c route/sch/prio.c route/sch/sfq.c route/sch/dsmark.c route/sch/tbf.c route/sch/netem.c
#CIN      += $(wildcard route/sch/*.c)
# Classifiers
#CIN      += $(wildcard route/cls/*.c)
#EGP
#CIN      += $(wildcard route/cls/egp/*.c)

DEPS     := $(CIN:%.c=.deps/%.d)
OBJ      := $(CIN:%.c=%.o)
CFLAGS   += -fPIC
OUT_SLIB := $(PACKAGE_NAME).so.$(PACKAGE_VERSION)
LN_SLIB  := $(PACKAGE_NAME).so
LN1_SLIB := $(LN_SLIB).0

export

.PHONY: all clean install librtn.a $(OUT_SLIB)


all:
	@echo "  MAKE $(OUT_SLIB)"; \
	$(MAKE) $(OUT_SLIB)

$(OUT_SLIB): ../Makefile.opts $(OBJ)
	@echo "  LD $(OUT_SLIB)"; \
	$(CC) -shared -Wl,-soname,libnl.so.0 -o $(OUT_SLIB) $(OBJ) $(LIBNL_LIB) -lc
	@echo "  LN $(OUT_SLIB) $(LN1_SLIB)"; \
	rm -f $(LN1_SLIB) ; $(LN) -s $(OUT_SLIB) $(LN1_SLIB)
	@echo "  LN $(LN1_SLIB) $(LN_SLIB)"; \
	rm -f $(LN_SLIB) ; $(LN) -s $(LN1_SLIB) $(LN_SLIB)

clean:
	@echo "  CLEAN lib"; \
	$(RM) -f $(OBJ) $(OUT_SLIB) $(LN_SLIB) $(LN1_SLIB); \
	$(RM) -f $(OUT_SLIB) $(LN_SLIB) $(LN1_SLIB)

distclean:
	find .deps -iname \*\.d -exec rm -f '{}' \;

install:
	mkdir -p $(DESTDIR)$(prefix)/lib/
	install -o root -g root -m 0644 $(OUT_SLIB) $(DESTDIR)$(prefix)/lib
	rm -f $(DESTDIR)$(prefix)/lib/$(LN1_SLIB) ; \
	$(LN) -s $(DESTDIR)$(prefix)/lib/$(OUT_SLIB) $(DESTDIR)$(prefix)/lib/$(LN1_SLIB)
	rm -f $(DESTDIR)$(prefix)/lib/$(LN_SLIB) ; \
	$(LN) -s $(DESTDIR)$(prefix)/lib/$(LN1_SLIB) $(DESTDIR)$(prefix)/lib/$(LN_SLIB)

$(DEPS): ../Makefile.opts

include ../Makefile.rules
