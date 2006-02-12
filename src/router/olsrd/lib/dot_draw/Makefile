# olsr.org dot-draw plugin by Andreas Tønnesen
#                             and Bruno Randolf
#
# $Id: Makefile,v 1.14 2005/03/31 17:09:53 kattemat Exp $

PLUGIN_NAME =	olsrd_dot_draw
PLUGIN_VER =	0.3

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
