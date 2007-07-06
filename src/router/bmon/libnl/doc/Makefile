#
# doc/Makefile
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

export

.PHONY: all gendoc clean distclean install

all:
	@true

gendoc:
	doxygen Doxyfile

clean:
	@true

distclean:
	$(RM) -f html/*

install:
	@true

$(DEPS): ../Makefile.opts
