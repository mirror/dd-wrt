######################
#
# Highlevel configuration options for all
#
# $Id: Makefile.inc,v 1.37 2007/10/20 20:23:12 bernd67 Exp $
#

# activate debugging with 1 or deactivate with 0
DEBUG ?= 0

# compile OLSR_PRINTF out
NO_DEBUG_MESSAGES ?= 0

# the optimize option to be set for gcc
OPTIMIZE ?= $(COPTS)

# enable mudflap with 1 or deactivate with 0
# you need a recent enough gcc and the libmudflap installed
MUDFLAP ?= 0


######################
#
# Lowlevel options and rules
#

# programs
CCACHE ?=       $(shell which ccache 2> /dev/null)
ifeq ($(origin CC),default)
CC = 		$(CCACHE) gcc
else
CC ?= 		$(CCACHE) gcc
endif
ifeq ($(DEBUG),0)
STRIP ?=	strip
else
STRIP ?=	:
endif
BISON ?=	bison
FLEX ?=		flex
TAGCMD ?=	etags

# target directories and names
DESTDIR ?=
SBINDIR ?= $(DESTDIR)/usr/sbin
ETCDIR  ?= $(DESTDIR)/etc
LIBDIR  ?= $(DESTDIR)/usr/lib
DOCDIR  ?= $(DESTDIR)/usr/share/doc
MANDIR  ?= $(DESTDIR)/usr/share/man
EXENAME ?= olsrd
CFGNAME ?= $(EXENAME).conf
CFGFILE ?= $(ETCDIR)/$(CFGNAME)

CPPFLAGS =	-Isrc
ifneq ($(TOPDIR),.)
CPPFLAGS +=	-I$(TOPDIR)/src
endif

# add gcc warnings and optimizations if CFLAGS not set
ifndef CFLAGS
ifndef WARNINGS
WARNINGS +=	-Wall
WARNINGS +=	-Wextra
WARNINGS +=	-Wold-style-definition
WARNINGS +=	-Wdeclaration-after-statement
WARNINGS +=	-Wmissing-prototypes
WARNINGS +=	-Wstrict-prototypes
WARNINGS +=	-Wmissing-declarations
WARNINGS +=	-Wsign-compare
WARNINGS +=	-Waggregate-return
WARNINGS +=	-Wmissing-noreturn
WARNINGS +=	-Wmissing-format-attribute
WARNINGS +=	-Wno-multichar
WARNINGS +=	-Wno-deprecated-declarations
#WARNINGS +=	-Wredundant-decls
ifeq ($(MUDFLAP),0)
# work around a bug in gcc-4.*
WARNINGS +=	-Wnested-externs
endif
# Alas, htons() triggers this so we can't seriously activate it.
#WARNINGS +=	-Wunreachable-code
WARNINGS +=	-Winline
WARNINGS +=	-Wdisabled-optimization
#WARNINGS +=	-Werror
WARNINGS +=	-finline-functions-called-once
WARNINGS +=	-fearly-inlining
ifeq ($(DEBUG),0)
WARNINGS +=	-fomit-framepointers
endif
# we have small inline functions in src/lq_route.c which should always be inlined
WARNINGS +=	-finline-limit=50
WARNINGS :=	$(shell CC="$(CC)" $(TOPDIR)/gcc-warnings $(WARNINGS))
endif
CFLAGS +=	$(WARNINGS)
CFLAGS +=	$(OPTIMIZE)
CFLAGS +=	$(EXTRA_CFLAGS)

# Must be specified along with -lpthread on linux
CPPFLAGS +=	$(OS_CFLAG_PTHREAD)
endif
ifneq ($(MUDFLAP),0)
CFLAGS +=	-fmudflapth
endif

ifdef OLSRD_PLUGIN
# c and ld flags for libraries (plugins)
CPPFLAGS +=	-DOLSR_PLUGIN
LDFLAGS +=	-shared -Wl,-soname,$(PLUGIN_NAME)
LDFLAGS +=	-Wl,--version-script=version-script.txt 
else
# c and ld flags for main
LDFLAGS +=	-Wl,-export-dynamic 
endif
ifneq ($(MUDFLAP),0)
LIBS +=		-lmudflapth
endif
LIBS +=		$(OS_LIB_PTHREAD)

###################################
#
# options to save space on small systems

# we have plugins with the old interface
#CPPFLAGS +=	-DSUPPORT_OLD_PLUGIN_VERSIONS=1

# search sources and headers in current dir and in src/
SRCS +=		$(wildcard src/*.c *.c)
HDRS +=		$(wildcard src/*.h *.h)

# OS detection
ifeq ($(OS),Windows_NT)
OS := win32
endif
ifeq ($(OS),)
OS := $(shell sh $(TOPDIR)/make/guess_os.sh)
endif
ifeq ($(OS),UNKNOWN)
all: help
else
# include OS specifics
all: default_target
include $(TOPDIR)/make/Makefile.$(OS)
endif

# one object for each source file
OBJS +=		$(SRCS:%.c=%.o)

# debugging or non-debugging flags
ifeq ($(DEBUG),1)
CPPFLAGS +=	-DDEBUG
CFLAGS +=	-g
else
CPPFLAGS +=	-DNDEBUG 
#CFLAGS += -ffunction-sections -fdata-sections -Wl,--gc-sections
endif
ifeq ($(NO_DEBUG_MESSAGES),1)
CPPFLAGS +=	-DNODEBUG
endif

# fully automatic and working dependency generation
%.d: %.c
	@$(CC) -M $(CPPFLAGS) "$<" | sed -e '1s|\($(*F)\)\.o[ :]*|$(*D)/\1.o $@: Makefile $(TOPDIR)$(if $(TOPDIR),/)Makefile.inc |g' >"$@"

# we always need the includes and defines
# for legacy since now
CPPFLAGS += $(INCLUDES) $(DEFINES)
ifneq ($(INCLUDES),)
$(warning Use CPPFLAGS instead of INCLUDES for -I)
endif
ifneq ($(DEFINES),)
$(warning Use CPPFLAGS instead of DEFINES for -D)
endif

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

ifeq ($(filter clean% %clean, $(MAKECMDGOALS)),)
# include dependencies
-include $(SRCS:%.c=%.d)
endif

# Local Variables:
# mode: makefile
# End:
