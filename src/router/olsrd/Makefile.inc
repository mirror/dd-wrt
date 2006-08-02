# programs
CC ?= 		gcc
STRIP ?=	strip
BISON ?=	bison
FLEX ?=		flex
TAGCMD ?=	etags

# target directories and names
#INSTALL_PREFIX = 
SBINDIR = $(INSTALL_PREFIX)/usr/sbin
ETCDIR = $(INSTALL_PREFIX)/etc
LIBDIR = $(INSTALL_PREFIX)/usr/lib
DOCDIR = $(INSTALL_PREFIX)/usr/share/doc
MANDIR = $(INSTALL_PREFIX)/usr/share/man
CFGFILE = $(ETCDIR)/olsrd.conf
EXENAME = olsrd

INCLUDES =	-Isrc -I$(TOPDIR)/src 

# add gcc warnings and optimizations if CFLAGS not set
ifndef CFLAGS
CFLAGS +=	-Wall -Wmissing-prototypes -Wstrict-prototypes \
		-Wmissing-declarations -Wsign-compare
CFLAGS +=	-O2 -g
endif

ifdef OLSRD_PLUGIN
# c and ld flags for libraries (plugins)
CFLAGS +=	-DOLSR_PLUGIN
LDFLAGS +=	-shared -Wl,-soname,$(PLUGIN_NAME)
LDFLAGS +=	-Wl,--version-script=version-script.txt 
else
# c and ld flags for main
LDFLAGS +=	-Wl,-export-dynamic 
endif

# search sources and headers in current dir and in src/
SRCS =		$(wildcard src/*.c)
SRCS +=		$(wildcard *.c)
HDRS =		$(wildcard src/*.h)
HDRS +=		$(wildcard *.h)

all: default_target

# OS detection
OS =		$(shell sh $(TOPDIR)/make/guess_os.sh)
ifeq ($(OS),UNKNOWN)
all: help
else
# include OS specifics
include $(TOPDIR)/make/Makefile.$(OS)
all: default_target
endif

# one object for each source file
OBJS +=		$(patsubst %.c,%.o,$(SRCS))

# debugging or non-debugging flags
ifdef DEBUG
CFLAGS +=	-DDEBUG
endif
ifdef NODEBUG
CFLAGS +=	-DNODEBUG
endif

# let gcc generate dependency information (*.d)
CFLAGS +=	-MD

# we always need the includes and defines
CFLAGS += $(INCLUDES) $(DEFINES)

TAGFILE ?=	src/TAGS

help:
	@echo 
	@echo '***** olsr.org olsr daemon Make ****'
	@echo ' Automatic detection of your OS     '
	@echo ' failed!                            '
	@echo ' You can provide a valid target OS  '
	@echo ' by setting the OS variable! Valid  '
	@echo ' target OSes are:                   '
	@echo ' ---------------------------------  '
	@echo ' linux - GNU/Linux                  '
	@echo ' win32 - MS Windows                 '
	@echo ' fbsd  - FreeBSD                    '
	@echo ' nbsd  - NetBSD                     '
	@echo ' obsd  - OpenBSD                    '
	@echo ' osx   - Mac OS X                   '
	@echo ' ---------------------------------  '
	@echo ' Example - build for windows:       '
	@echo ' make OS=win32                      '
	@echo ' If you are developing olsrd code,  '
	@echo ' exporting the OS variable might    '
	@echo ' be a good idea :-) Have fun!       '
	@echo '************************************'
	@echo
	
# include dependencies
-include $(SRCS:%.c=%.d)
