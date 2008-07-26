######################
#
# Highlevel configuration options for all
#
#

# activate debugging with 1 or deactivate with 0
DEBUG = 0

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
CCACHE ?=       $(shell IFS=:; for i in $$PATH;do test -x "$$i/ccache" && echo "$$i/ccache" && break; done)
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
# use the new fixed point math stuff
CPPFLAGS +=     -DUSE_FPM

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
WARNINGS +=	-Wendif-labels
WARNINGS +=	-Wwrite-strings
WARNINGS +=	-Wbad-function-cast
WARNINGS +=	-Wpointer-arith
WARNINGS +=	-Wcast-qual
# the following 5 do not work yet and need more work on it
#WARNINGS +=	-Wshadow
#WARNINGS +=	-Wpointer-arith
#WARNINGS +=	-Wcast-align
#WARNINGS +=	-Wconversion
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
WARNINGS +=	-fomit-frame-pointer
endif
# we have small inline functions in src/lq_route.h which should always be inlined
WARNINGS +=	-finline-limit=50
# These tell gcc to put each function and global variable in a separate section.
# The linker can than remove all unreferenced section. But in the olsrd binary
# unused doesn't imply unused at all since the function may be used by plugins,
# e.g. the ones in src/plugin_utils.c.
# So we can use that featuer at most to identify unused functions and remove them
# from the source by hand.
#WARNINGS +=	-ffunction-sections
#WARNINGS +=	-fdata-sections
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
LDFLAGS +=	-shared
LDFLAGS +=	-Wl,-soname,$(PLUGIN_NAME)
LDFLAGS +=	-Wl,--version-script=version-script.txt 
else
# c and ld flags for main
LDFLAGS +=	-Wl,-export-dynamic 
endif
# LDFLAGS for all 
LDFLAGS +=	-Wl,--warn-common
# See above at "-ffunction-sections" for an explanation (and why it is disabled).
#LDOPTS +=	-Wl,--gc-sections
#LDOPTS +=	-Wl,--print-gc-sections
#LDFLAGS +=	$(shell CC="$(CC)" $(TOPDIR)/ld-warnings $(LDOPTS))
ifneq ($(MUDFLAP),0)
LIBS +=		-lmudflapth
endif
LIBS +=		$(OS_LIB_PTHREAD)

# extra options from the outside
CPPFLAGS +=	$(EXTRA_CPPFLAGS)

###################################
#
# options to save space on small systems

# we have plugins with the old interface
#CPPFLAGS +=	-DSUPPORT_OLD_PLUGIN_VERSIONS=1

# search sources and headers in current dir and in src/
SRCS +=		$(wildcard src/common/*.c src/*.c *.c)
HDRS +=		$(wildcard src/common/*.h src/*.h *.h)

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
CPPFLAGS +=	-DDEBUG -DNEED_PRINTF
CFLAGS +=	-g
else
CPPFLAGS +=	-DNDEBUG
endif
ifeq ($(NO_DEBUG_MESSAGES),1)
CPPFLAGS +=	-DNODEBUG
endif

# a make function to quote "/" and "."
quote = $(subst .,\.,$(subst /,\/,$1))

# fully automatic and working dependency generation
%.d: %.c
	@$(filter-out $(CCACHE),$(CC)) -M $(strip $(CPPFLAGS)) "$<" | sed -e '1s/\($(call quote,$(*F))\.o\)[ :]*/$(call quote,$(*D)/\1 $@: Makefile $(TOPDIR)$(if $(TOPDIR),/)Makefile.inc) /g' >"$@"

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
# include dependencies - we don't need any dependency for a everytime generated files
-include $(filter-out src/builddata.%,$(SRCS:%.c=%.d))
endif

# Local Variables:
# mode: makefile
# End:
