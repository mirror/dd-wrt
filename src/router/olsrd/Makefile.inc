# The olsr.org Optimized Link-State Routing daemon (olsrd)
#
# (c) by the OLSR project
#
# See our Git repository to find out who worked on this file
# and thus is a copyright holder on it.
#
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

######################
#
# Highlevel configuration options for all
#
#

# activate debugging with 1 or deactivate with 0
DEBUG = 0

# set NOSTRIP to non-zero to inhibit stripping the binaries
NOSTRIP ?= 0

# compile OLSR_PRINTF out
NO_DEBUG_MESSAGES = 1

include $(TOP)/.config

# the optimize option to be set for gcc
OPTIMIZE = $(COPTS) $(MIPS16_OPT) -D_GNU_SOURCE $(LTO)

ifeq ($(CONFIG_DIST),"micro")
OPTIMIZE += -DNEED_NOPRINTF
CFLAGS += -DNEED_NOPRINTF
endif
ifneq ($(CONFIG_DIST),"micro-special")
OPTIMIZE += -DNEED_NOPRINTF
CFLAGS += -DNEED_NOPRINTF
endif

# enable mudflap with 1 or deactivate with 0
# you need a recent enough gcc and the libmudflap installed
MUDFLAP ?= 0

# shows full compiler/linker calls if activated
VERBOSE ?= 0

SANITIZE_ADDRESS   ?= 0
SANITIZE_LEAK      ?= 0
SANITIZE_UNDEFINED ?= 0

ifeq ($(VERBOSE),0)
MAKECMDPREFIX = @
else
MAKECMDPREFIX =
endif

# OS detection
ifeq ($(OS),Windows_NT)
OS := win32
endif
ifeq ($(OS),)
OS := $(shell sh $(TOPDIR)/make/guess_os.sh)
endif

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
CC := $(strip $(CC))
ifeq ($(DEBUG),0)
  ifeq ($(NOSTRIP),0)
  STRIP ?=  strip
  else
  STRIP ?=  :
  endif
else
STRIP ?=  :
endif
BISON ?=	bison
FLEX ?=		flex
TAGCMD ?=	etags

# target directories and names
DESTDIR  ?=
ETCDIR   ?= $(DESTDIR)/etc/olsrd
USRDIR   ?= $(DESTDIR)/usr
INCLUDEDIR ?= $(DESTDIR)/usr/include
LIBDIR   ?= $(USRDIR)/lib
SBINDIR  ?= $(USRDIR)/sbin
SHAREDIR ?= $(USRDIR)/share
DOCDIR   ?= $(SHAREDIR)/doc
MANDIR   ?= $(SHAREDIR)/man
EXENAME  ?= olsrd
CFGNAME  ?= $(EXENAME).conf
CFGFILE  ?= $(ETCDIR)/$(CFGNAME)
SGW_POLICY_SCRIPT ?= sgw_policy_routing_setup.sh

DOCDIR_OLSRD ?= $(DOCDIR)/$(EXENAME)


CPPFLAGS  = -I$(TOPDIR)/src -I$(TOPDIR)/lib -I$(TOPDIR)/lib/pud/nmealib/include -I$(TOPDIR)/lib/pud/wireformat/include -D_GNU_SOURCE $(LTO)

# include OS specific compiler settings
-include $(TOPDIR)/make/Makefile.$(OS).compiler

# add gcc warnings and optimizations if CFLAGS not set
ifndef CFLAGS
ifndef WARNINGS
#
# warning flags to add if supported (sorted)
#
WARNINGS += -fearly-inlining
WARNINGS += -finline-functions-called-once
WARNINGS += -funit-at-a-time
WARNINGS += -Waggregate-return
WARNINGS += -Wall
WARNINGS += -Wbad-function-cast
WARNINGS += -Wcast-align
WARNINGS += -Wdeclaration-after-statement
WARNINGS += -Wdisabled-optimization
WARNINGS += -Wendif-labels
#WARNINGS += -Werror
WARNINGS += -Werror=format-security
WARNINGS += -Wextra
WARNINGS += -Wformat
WARNINGS += -Wformat-security
WARNINGS += -Wformat-y2k
WARNINGS += -Winit-self
WARNINGS += -Winline
WARNINGS += -Wmissing-declarations
WARNINGS += -Wmissing-format-attribute
WARNINGS += -Wmissing-noreturn
WARNINGS += -Wmissing-prototypes
WARNINGS += -Wno-deprecated-declarations
WARNINGS += -Wno-multichar
WARNINGS += -Wnull-dereference
WARNINGS += -Wold-style-definition
WARNINGS += -Wpointer-arith
WARNINGS += -Wsequence-point
WARNINGS += -Wshadow
WARNINGS += -Wshift-negative-value
WARNINGS += -Wshift-overflow
WARNINGS += -Wsign-compare
WARNINGS += -Wstrict-prototypes
WARNINGS += -Wswitch-default
WARNINGS += -Wtautological-compare
WARNINGS += -Wundef
WARNINGS += -Wunused-parameter
WARNINGS += -Wwrite-strings

ifneq ($(CC),clang)
# clang doesn't report these as errors or warnings in the gcc-warnings script
WARNINGS += -Wdouble-promotion
WARNINGS += -Wduplicated-cond
WARNINGS += -Wjump-misses-init
WARNINGS += -Wlogical-op
WARNINGS += -Wmisleading-indentation
WARNINGS += -Wsync-nand
WARNINGS += -Wtrampolines
endif

ifneq ($(OS),win32)
# triggers many warnings for inet_ntop in src/win32/compat.h
WARNINGS += -Wcast-qual
endif

# we have small inline functions in src/lq_route.h which should always be inlined
WARNINGS += -finline-limit=350

# only in non-debug builds
ifeq ($(DEBUG),0)
WARNINGS += -fomit-frame-pointer
endif

# the following 2 do not work yet and need more work on it
#WARNINGS += -Wconversion
#WARNINGS += -Wredundant-decls

# Alas, htons() triggers this so we can't seriously activate it.
#WARNINGS += -Wunreachable-code

# These tell gcc to put each function and global variable in a separate section.
# The linker can then remove all unreferenced sections. But in the olsrd binary
# unused doesn't imply unused at all since the function may be used by plugins,
# e.g. the ones in src/plugin_utils.c.
# So we can use that featuer at most to identify unused functions and remove
# them from the source by hand.
#WARNINGS += -ffunction-sections
#WARNINGS += -fdata-sections

ifeq ($(MUDFLAP),0)
# work around a bug in gcc-4.*
WARNINGS += -Wnested-externs
endif

WARNINGS :=	$(shell CC="$(CC)" "$(TOPDIR)/gcc-warnings" $(WARNINGS))
endif
CFLAGS +=	$(WARNINGS)
CFLAGS +=	$(OPTIMIZE)
CPPFLAGS +=	$(OPTIMIZE)
CFLAGS +=	$(EXTRA_CFLAGS)

endif
ifneq ($(MUDFLAP),0)
CFLAGS +=	-fmudflapth
endif

ifdef OLSRD_PLUGIN
# c and ld flags for libraries (plugins)
CPPFLAGS +=	-DOLSR_PLUGIN
ifeq ($(OS),win32)
  LDFLAGS +=	-Wl,-export-all-symbols,--enable-auto-import
endif
LDFLAGS +=	-shared
LDFLAGS +=	-Wl,-soname,$(PLUGIN_SONAME)
LDFLAGS +=	-Wl,--version-script=version-script.txt 
else
# c and ld flags for main
ifeq ($(OS),win32)
  LDFLAGS +=	-Wl,-export-all-symbols
else 
  LDFLAGS +=	-Wl,-export-dynamic 
endif
LDFLAGS +=	-Wl,-rpath,$(LIBDIR)
endif
# LDFLAGS for all 
LDFLAGS +=	-Wl,--warn-common
LDFLAGS +=	$(LDLTO)
# See above at "-ffunction-sections" for an explanation (and why it is disabled).
#LDOPTS +=	-Wl,--gc-sections
#LDOPTS +=	-Wl,--print-gc-sections
#LDFLAGS +=	$(shell CC="$(CC)" $(TOPDIR)/ld-warnings $(LDOPTS))
ifneq ($(MUDFLAP),0)
LIBS +=		-lmudflapth
endif

ifeq ($(OS),linux)
  ifneq ($(SANITIZE_ADDRESS),0)
    CFLAGS  += -fsanitize=address -fsanitize-recover -fno-omit-frame-pointer
    LDFLAGS += -fsanitize=address -fsanitize-recover -static-libasan
  endif
  ifneq ($(SANITIZE_LEAK),0)
    CFLAGS  += -fsanitize=leak -fsanitize-recover
    LDFLAGS += -fsanitize=leak -fsanitize-recover -static-liblsan
  endif
  ifneq ($(SANITIZE_UNDEFINED),0)
    CFLAGS  += -fsanitize=undefined -fsanitize-recover
    LDFLAGS += -fsanitize=undefined -fsanitize-recover -static-libubsan
  endif
endif

# extra options from the outside
CPPFLAGS +=	$(EXTRA_CPPFLAGS)


# 32/64 cross compilation
ifdef M32
CFLAGS +=	-m32
LDFLAGS +=	-m32
endif
ifdef M64
CFLAGS +=	-m64
LDFLAGS +=	-m64
endif


###################################
#
# options to save space on small systems

# we have plugins with the old interface
#CPPFLAGS +=	-DSUPPORT_OLD_PLUGIN_VERSIONS=1

# use the new fixed point math stuff
CPPFLAGS +=     -DUSE_FPM

# search sources and headers in current dir and in src/
SRCS +=		$(sort $(wildcard src/common/*.c src/*.c *.c))
HDRS +=		$(sort $(wildcard src/common/*.h src/*.h *.h))

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
ifeq ($(OS),linux)
LDFLAGS  += -rdynamic
endif
else
CPPFLAGS +=	-DNDEBUG
ifeq ($(OPTIMIZE),)
OPTIMIZE += -O2 -D_FORTIFY_SOURCE=2 
endif
endif
ifeq ($(NO_DEBUG_MESSAGES),1)
CPPFLAGS +=	-DNODEBUG
endif

# preserve debugging info when NOSTRIP is set
ifneq ($(NOSTRIP),0)
CFLAGS +=	-ggdb
endif

# a make function to quote "/" and "."
quote = $(subst .,\.,$(subst /,\/,$1))

# fully automatic and working dependency generation
%.d: %.c
	$(MAKECMDPREFIX)$(filter-out $(CCACHE),$(CC)) -M $(strip $(CPPFLAGS)) "$<" | sed -e '1s/\($(call quote,$(*F))\.o\)[ :]*/$(call quote,$(*D)/\1 $@: Makefile $(TOPDIR)$(if $(TOPDIR),/)Makefile.inc) /g' >"$@"

%.o: %.c
ifeq ($(VERBOSE),0)
	@echo "[CC] $<"
endif
	$(MAKECMDPREFIX)$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

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
	@echo ' linux   - GNU/Linux                '
	@echo ' win32   - MS Windows               '
	@echo ' fbsd    - FreeBSD                  '
	@echo ' nbsd    - NetBSD                   '
	@echo ' obsd    - OpenBSD                  '
	@echo ' osx     - Mac OS X                 '
	@echo ' android - Android                  '
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
