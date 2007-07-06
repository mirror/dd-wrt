#
# lib/Makefile
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

# Core
CIN      := $(wildcard *.c)
# NETLINK_ROUTE
CIN      += $(wildcard route/*.c)
# Schedulers
CIN      += $(wildcard route/sch/*.c)
# Classifiers
CIN      += $(wildcard route/cls/*.c)
# fib lookup
CIN      += $(wildcard fib_lookup/*.c)

DEPS     := $(CIN:%.c=.deps/%.d)
OBJ      := $(CIN:%.c=%.o)
CFLAGS   += -fPIC
OUT_SLIB := $(PACKAGE_NAME).so.$(PACKAGE_VERSION)
LN_SLIB  := $(PACKAGE_NAME).so
LN1_SLIB := $(LN_SLIB).1

export

.PHONY: all clean install librtn.a $(OUT_SLIB)


all:
	@echo "  MAKE $(OUT_SLIB)"; \
	$(MAKE) $(OUT_SLIB)

$(OUT_SLIB): ../Makefile.opts $(OBJ)
	@echo "  LD $(OUT_SLIB)"; \
	$(CC) -shared -Wl,-soname,libnl.so.1 -o $(OUT_SLIB) $(OBJ) $(LIBNL_LIB) -lc
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
	mkdir -p $(INSTALLDIR)/usr/lib
	install -m 0644 $(OUT_SLIB) $(INSTALLDIR)/usr/lib
	rm -f $(INSTALLDIR)/usr/lib$(LN1_SLIB)
	$(LN) -s $(OUT_SLIB) $(INSTALLDIR)/usr/lib/$(LN1_SLIB)
	rm -f $(INSTALLDIR)/usr/lib$(LN_SLIB)
	$(LN) -s $(LN1_SLIB) $(INSTALLDIR)/usr/lib/$(LN_SLIB)

$(DEPS): ../Makefile.opts

include ../Makefile.rules
