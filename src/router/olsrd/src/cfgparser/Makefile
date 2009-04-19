
# The olsr.org Optimized Link-State Routing daemon(olsrd)
# Copyright (c) 2004, Andreas Tonnesen(andreto@olsr.org)
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

TOPDIR=../..
include $(TOPDIR)/Makefile.inc

# delete the variables from above
SRCS =
HDRS =

include local.mk

ifeq ($(OS), win32)

LIBNAME ?=	olsrd_cfgparser.dll
BINNAME ?=	olsrd_cfgparser.exe
OBJS +=		../win32/compat.o ../ipcalc.o ../builddata.o

else

LIBNAME ?=	olsrd_cfgparser.so.0.1
BINNAME ?=	olsrd_cfgparser
endif


ifdef MAKEBIN

CPPFLAGS +=	-DMAKEBIN 
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

CPPFLAGS +=	-DMAKELIB
NAME =		$(LIBNAME)

endif

.PHONY: install clean uberclean

# Targets
default_target: $(NAME)

$(NAME):	$(OBJS)
		$(CC) $(LDFLAGS) -o "$@" $^ $(LIBS)

install:
		install -D -m 755 $(NAME) $(LIBDIR)/$(NAME)
		/sbin/ldconfig -n $(LIBDIR)

clean:
		rm -f *.[od~] $(LIBNAME) $(BINNAME) $(TMPFILES)

uberclean:	clean
