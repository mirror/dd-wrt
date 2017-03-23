# Makefile settings shared between Makefiles.

CC = ccache ccache arm-linux-uclibc-gcc
CXX = ccache arm-linux-uclibc-g++
CFLAGS = -fPIC -DNEED_PRINTF -Os -pipe -march=armv7-a -mcpu=cortex-a9 -mtune=cortex-a9  -msoft-float -mfloat-abi=soft -fno-caller-saves -fno-plt -DHAVE_NVRAM_128  -I/opt/DEV/src/router/pcre -I/opt/DEV/src/router/gmp -I/opt/DEV/src/router/zlib -ggdb3 -Wall -W   -Wmissing-prototypes -Wmissing-declarations -Wstrict-prototypes   -Wpointer-arith -Wbad-function-cast -Wnested-externs
CXXFLAGS = -g -O2
CCPIC = -fpic
CPPFLAGS = -Os -pipe -march=armv7-a -mcpu=cortex-a9 -mtune=cortex-a9  -msoft-float -mfloat-abi=soft -fno-caller-saves -fno-plt -DHAVE_NVRAM_128  -I/opt/DEV/src/router/openssl/include
DEFS = -DHAVE_CONFIG_H
LDFLAGS = -L/opt/DEV/src/router/pcre/.libs -L/opt/DEV/src/router/gmp/.libs -lpthread -lpcre -L/opt/DEV/src/router/zlib  -lz
LIBS = -lgmp 
LIBOBJS = 
EMULATOR = 
NM = arm-linux-uclibc-nm

OBJEXT = o
EXEEXT = 

CC_FOR_BUILD = gcc -O
EXEEXT_FOR_BUILD = 

DEP_FLAGS = -MT $@ -MD -MP -MF $@.d
DEP_PROCESS = true

PACKAGE_BUGREPORT = nettle-bugs@lists.lysator.liu.se
PACKAGE_NAME = nettle
PACKAGE_STRING = nettle 3.3
PACKAGE_TARNAME = nettle
PACKAGE_VERSION = 3.3

LIBNETTLE_MAJOR = 6
LIBNETTLE_MINOR = 3
LIBNETTLE_SONAME = $(LIBNETTLE_FORLINK).$(LIBNETTLE_MAJOR)
LIBNETTLE_FILE = $(LIBNETTLE_SONAME).$(LIBNETTLE_MINOR)
LIBNETTLE_FILE_SRC = $(LIBNETTLE_FORLINK)
LIBNETTLE_FORLINK = libnettle.so
LIBNETTLE_LIBS = 
LIBNETTLE_LINK = $(CC) $(CFLAGS) $(LDFLAGS) -shared -Wl,-soname=$(LIBNETTLE_SONAME)

LIBHOGWEED_MAJOR = 4
LIBHOGWEED_MINOR = 3
LIBHOGWEED_SONAME = $(LIBHOGWEED_FORLINK).$(LIBHOGWEED_MAJOR)
LIBHOGWEED_FILE = $(LIBHOGWEED_SONAME).$(LIBHOGWEED_MINOR)
LIBHOGWEED_FILE_SRC = $(LIBHOGWEED_FORLINK)
LIBHOGWEED_FORLINK = libhogweed.so
LIBHOGWEED_LIBS = libnettle.so $(LIBS)
LIBHOGWEED_LINK = $(CC) $(CFLAGS) $(LDFLAGS) -shared -Wl,-soname=$(LIBHOGWEED_SONAME)

GMP_NUMB_BITS = 32

AR = arm-linux-uclibc-ar
ARFLAGS = cru
AUTOCONF = autoconf
AUTOHEADER = autoheader
M4 = /usr/bin/m4
MAKEINFO = makeinfo
RANLIB = arm-linux-uclibc-ranlib
LN_S = ln -s

prefix	=	/usr
exec_prefix =	${prefix}
datarootdir =	${prefix}/share
bindir =	${exec_prefix}/bin
libdir =	${exec_prefix}/lib
includedir =	${prefix}/include
infodir =	${datarootdir}/info

# PRE_CPPFLAGS and PRE_LDFLAGS lets each Makefile.in prepend its own
# flags before CPPFLAGS and LDFLAGS. While EXTRA_CFLAGS are added at the end.

COMPILE = $(CC) $(PRE_CPPFLAGS) $(CPPFLAGS) $(DEFS) $(CFLAGS) $(EXTRA_CFLAGS) $(DEP_FLAGS)
COMPILE_CXX = $(CXX) $(PRE_CPPFLAGS) $(CPPFLAGS) $(DEFS) $(CXXFLAGS) $(DEP_FLAGS)
LINK = $(CC) $(CFLAGS) $(PRE_LDFLAGS) $(LDFLAGS)
LINK_CXX = $(CXX) $(CXXFLAGS) $(PRE_LDFLAGS) $(LDFLAGS)

# Default rule. Must be here, since config.make is included before the
# usual targets.
default: all

# For some reason the suffixes list must be set before the rules.
# Otherwise BSD make won't build binaries e.g. aesdata. On the other
# hand, AIX make has the opposite idiosyncrasies to BSD, and the AIX
# compile was broken when .SUFFIXES was moved here from Makefile.in.

.SUFFIXES:
.SUFFIXES: .asm .c .$(OBJEXT) .html .dvi .info .exe .pdf .ps .texinfo

# Disable builtin rule
%$(EXEEXT) : %.c
.c:

# Keep object files
.PRECIOUS: %.o

.PHONY: all check install uninstall clean distclean mostlyclean maintainer-clean distdir \
	all-here check-here install-here clean-here distclean-here mostlyclean-here \
	maintainer-clean-here distdir-here \
	install-shared install-info install-headers \
	uninstall-shared uninstall-info uninstall-headers \
	dist distcleancheck
