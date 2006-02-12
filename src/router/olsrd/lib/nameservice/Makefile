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
# $Id: Makefile,v 1.9 2005/03/01 21:41:34 tlopatic Exp $

PLUGIN_NAME =	olsrd_nameservice
PLUGIN_VER =	0.2

CC ?=		gcc
STRIP ?=	strip

# default CFLAGS and LDFLAGS, used if not externally set

# used for compilation: yes
# used for dependency file generation: no

CFLAGS_SET =	-g -O2 -Wall -Wmissing-prototypes -Wstrict-prototypes
LDFLAGS_SET =	-g

# always appended to default or externally set CFLAGS and LDFLAGS

# used for compilation: yes
# used for dependency file generation: yes

CFLAGS_ADD =	-DOLSR_PLUGIN -I../../src
LDFLAGS_ADD =	-Wall -shared -Wl,--version-script=version-script.txt

# same as CFLAGS_ADD, but not used for dependency file generation

# used for compilation: yes
# used for dependency file generation: no

CFLAGS_ADD2 =	# nothing

DEPFILE =	.depend

SRCS =		$(wildcard src/*.c)
OBJS =		$(patsubst %.c,%.o,$(SRCS))
HDRS =		$(wildcard src/*.c)

ifndef OS
all:		help
install:	help
clean:		help
endif

ifeq ($(OS), linux)

NAME ?=		$(PLUGIN_NAME).so.$(PLUGIN_VER)

CFLAGS_ADD +=	-Dlinux
CFLAGS_ADD2 +=	-fPIC
LDFLAGS_ADD +=	-fPIC -Wl,-soname,$(NAME)

CFLAGS_SET +=	# nothing
LDFLAGS_SET +=	# nothing

LIBS ?=		-lc -lm

LIBDIR ?=	$(INSTALL_PREFIX)/usr/lib

INSTALL_LIB =	install -D -m 755 $(NAME) $(LIBDIR)/$(NAME); \
		/sbin/ldconfig -n $(LIBDIR)

EXTRA_OBJS =	# nothing

MAKEDEPEND = 	makedepend -f $(DEPFILE) $(CFLAGS_ADD) -Y $(INCLUDES) \
		$(SRCS) >/dev/null 2>&1

all:		all2
install:	install2
clean:		clean2

else
ifeq ($(OS), fbsd)

NAME ?=		$(PLUGIN_NAME).so.$(PLUGIN_VER)

CFLAGS_ADD +=	# nothing
CFLAGS_ADD2 +=	-fPIC
LDFLAGS_ADD +=	-fPIC -Wl,-soname,$(NAME)

CFLAGS_SET +=	# nothing
LDFLAGS_SET +=	# nothing

LIBS ?=		-lc -lm

LIBDIR ?=	$(INSTALL_PREFIX)/usr/lib

INSTALL_LIB =	install -m 755 $(NAME) $(LIBDIR)/$(NAME); \
		/sbin/ldconfig

EXTRA_OBJS =	# nothing

MAKEDEPEND = 	makedepend -f $(DEPFILE) $(CFLAGS_ADD) $(INCLUDES) $(SRCS)

all:		all2
install:	install2
clean:		clean2

else
ifeq ($(OS), win32)

NAME ?=		$(PLUGIN_NAME).dll

CFLAGS_ADD +=	-mno-cygwin -I../../src/win32 -DWIN32
CFLAGS_ADD2 +=	# nothing
LDFLAGS_ADD +=	-mno-cygwin -Wl,-soname,$(NAME)

CFLAGS_SET +=	# nothing
LDFLAGS_SET +=	# nothing

LIBS ?=		-lws2_32

INSTALL_LIB =	cp $(NAME) ../..

EXTRA_OBJS =	../../src/win32/compat.o

MAKEDEPEND = 	makedepend -f $(DEPFILE) $(CFLAGS_ADD) $(INCLUDES) $(SRCS)

all:		all2
install:	install2
clean:		clean2

else

all:		help
install:	help
clean:		help

endif
endif
endif

CFLAGS ?=	$(CFLAGS_SET)
LDFLAGS ?=	$(LDFLAGS_SET)

CFLAGS		+= $(CFLAGS_ADD) $(CFLAGS_ADD2)
LDFLAGS		+= $(LDFLAGS_ADD)

all2:		$(NAME)

$(NAME):	$(OBJS) $(EXTRA_OBJS)
		$(CC) $(LDFLAGS) -o $(NAME) $(OBJS) $(EXTRA_OBJS) $(LIBS)

install2:	$(NAME)
		$(STRIP) $(NAME)
		$(INSTALL_LIB)

clean2:
		rm -f $(OBJS) $(NAME) $(DEPFILE)

$(DEPFILE):	$(SRCS) $(HDRS)
ifdef MAKEDEPEND
		echo "# Generated automatically. DO NOT EDIT." >$(DEPFILE)
		$(MAKEDEPEND)
endif

help:
		@echo
		@echo '                     * * * *  olsr.org Plugin Make  * * * *'
		@echo
		@echo '  Plugin: $(PLUGIN_NAME), version $(PLUGIN_VER)'
		@echo
		@echo '  You must provide a valid target OS by setting the OS variable.'
		@echo
		@echo '  Valid settings are:'
		@echo
		@echo '    linux - GNU/Linux'
		@echo '    win32 - Microsoft Windows'
		@echo '    fbsd  - FreeBSD'
		@echo
		@echo '  Example - Build on Windows:'
		@echo
		@echo '    make OS=win32'
		@echo
		@echo '  Example - Build and install on Linux:'
		@echo
		@echo '    make OS=linux install            '
		@echo

sinclude $(DEPFILE)
