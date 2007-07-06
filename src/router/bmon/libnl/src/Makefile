#
# src/Makefile
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

CFLAGS	+= -L../lib
CIN 	:= $(wildcard nl-*.c)
TOOLS	:= $(CIN:%.c=%)

all: $(TOOLS)

$(TOOLS): utils.o

nl-%: nl-%.c
	$(CC) $(CFLAGS) -o $@ $< -lnl utils.o

clean:
	rm -f $(TOOLS) utils.o

distclean: clean

install:
	@true
