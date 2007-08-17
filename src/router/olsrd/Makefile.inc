# programs
CCACHE ?=       $(shell sh -c "type -path ccache")
ifeq ($(origin CC),default)
CC = 		$(CCACHE) gcc
else
CC ?= 		$(CCACHE) gcc
endif
STRIP ?=	strip
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
CFGFILE ?= $(ETCDIR)/olsrd.conf
EXENAME ?= olsrd

CPPFLAGS =	-Isrc -I$(TOPDIR)/src 

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
WARNINGS +=	-Wnested-externs
#WARNINGS +=	-Wunreachable-code
WARNINGS +=	-Winline
WARNINGS +=	-Wdisabled-optimization
#WARNINGS +=	-Werror
WARNINGS :=	$(shell CC="$(CC)" $(TOPDIR)/gcc-warnings $(WARNINGS))
endif
CFLAGS +=	$(WARNINGS)
CFLAGS +=	-O2
CFLAGS +=	-g
endif
CFLAGS +=	-DNODEBUG

ifdef OLSRD_PLUGIN
# c and ld flags for libraries (plugins)
CPPFLAGS +=	-DOLSR_PLUGIN
LDFLAGS +=	-shared -Wl,-soname,$(PLUGIN_NAME)
LDFLAGS +=	-Wl,--version-script=version-script.txt 
else
# c and ld flags for main
LDFLAGS +=	-Wl,-export-dynamic 
endif

###################################
#
# options to save space on small systems

# we have plugins with the old interface
CPPFLAGS +=	-DSUPPORT_OLD_PLUGIN_VERSIONS=1

# search sources and headers in current dir and in src/
SRCS +=		$(wildcard src/*.c *.c)
HDRS +=		$(wildcard src/*.h *.h)

all: default_target

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
include $(TOPDIR)/make/Makefile.$(OS)
all: default_target
endif

# one object for each source file
OBJS +=		$(SRCS:%.c=%.o)

# debugging or non-debugging flags
ifdef DEBUG
CPPFLAGS +=	-DDEBUG
endif
ifdef NODEBUG
CPPFLAGS +=	-DNODEBUG
endif

# fully automatic and working dependency generation
%.d: %.c
	@$(CC) -M $(CPPFLAGS) "$<" | sed -e '1s|\($(*F)\)\.o[ :]*|$(*D)/\1.o $@: Makefile $(TOPDIR)$(if $(TOPDIR),/)Makefile.inc \\\n  |g' >"$@"

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
