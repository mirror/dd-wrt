
# The olsr.org Optimized Link-State Routing daemon(olsrd)
# Copyright (c) 2004, Andreas Tønnesen(andreto@olsr.org)
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without 
# modification, are permitted provided that the following conditions 
# are met:
#
# * Redistributions of source code must retain the above copyright 
#   notice, this list of conditions and the following disclaimer.
# * Redistributions in binary form must reproduce the above copyright 
#   notice, this list of conditions and the following disclaimer in 
#   the documentation and/or other materials provided with the 
#   distribution.
# * Neither the name of olsr.org, olsrd nor the names of its 
#   contributors may be used to endorse or promote products derived 
#   from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE 
# COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN 
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
# POSSIBILITY OF SUCH DAMAGE.
#
# Visit http://www.olsr.org for more information.
#
# If you find this software useful feel free to make a donation
# to the project. For more information see the website or contact
# the copyright holders.
#
# $Id: Makefile,v 1.25 2007/04/27 11:32:27 bernd67 Exp $

TOPDIR=../..
include $(TOPDIR)/Makefile.inc

SRCS =		olsrd_conf.c  oparse.c  oscan.c cfgfile_gen.c
HDRS =		olsrd_conf.h  oparse.h

ifeq ($(OS), win32)

LIBNAME ?=	olsrd_cfgparser.dll
BINNAME ?=	olsrd_cfgparser.exe
CFLAGS +=	-DWIN32_STDIO_HACK
OBJS +=		../win32/compat.o

else

LIBNAME ?=	olsrd_cfgparser.so.0.1
BINNAME ?=	olsrd_cfgparser

endif


ifdef MAKEBIN

CFLAGS +=	-DMAKEBIN 
NAME =		$(BINNAME)

else 

# build lib per default

ifeq (${OS}, osx)
LDFLAGS +=	-dynamiclib -single_module
else
LDFLAGS +=	-shared -Wl,-soname,$(LIBNAME)
endif

ifeq (${OS}, win32)
LDFLAGS +=	-Wl,--out-implib=libolsrd_cfgparser.a -Wl,--export-all-symbols
endif

CFLAGS +=	-DMAKELIB
NAME =		$(LIBNAME)

endif


default_target: $(NAME)

$(NAME):	$(OBJS)
		$(CC) $(LDFLAGS) -o $(NAME) $(OBJS) $(LIBS)

oscan.c:	oscan.lex Makefile
		$(FLEX) -Cem -ooscan.c-tmp oscan.lex
		sed	-e '/^static/s/yy_get_next_buffer[\(][\)]/yy_get_next_buffer(void)/' \
			-e '/^static/s/yy_get_previous_state[\(][\)]/yy_get_previous_state(void)/' \
			-e '/^static/s/input[\(][\)]/input(void)/' \
			-e '/^static  *void  *yy_fatal_error/s/^\(.*)\);$$/\1 __attribute__((noreturn));/' \
			< oscan.c-tmp >oscan.c
		$(RM) oscan.c-tmp

oparse.c:	oparse.y olsrd_conf.h Makefile
		$(BISON) -d -o oparse.c oparse.y

#oscan.o oparse.o:: CFLAGS := $(filter-out -Wunreachable-code, $(CFLAGS))

.PHONY: install clean uberclean

install:
		install -D -m 755 $(NAME) $(LIBDIR)/$(NAME)
		/sbin/ldconfig -n $(LIBDIR)

clean:
		rm -f *.[od~] oscan.c oparse.[ch]
		rm -f $(LIBNAME)
		rm -f $(BINNAME)
		$(RM) oparse.c-tmp

uberclean:	clean
		rm -f oparse.c oparse.h oscan.c
