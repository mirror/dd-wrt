#
#	MatrixSSL example applications makefile
#	Builds optimized by default, see comments below for debug.
#	Copyright (c) PeerSec Networks, 2002-2007. All Rights Reserved
#

CC 	= gcc
O 	= .o
SO  = .so
A   = .a
E	=

LIBRARIES	= ../src/libmatrixssl$(SO)
STATICS		= ../src/libmatrixsslstatic$(A)

REF_EXE 	= httpsReflector$(E)
REF_OBJS 	= httpsReflector$(O) sslSocket$(O)

CLIENT_EXE	= httpsClient$(E)
CLIENT_OBJS = httpsClient$(O) sslSocket$(O)

#
#	Debug flags (Define MATRIXSSL_DEBUG in shell to build debug) 
#	bash:	export MATRIXSSL_DEBUG=1
#			unset MATRIXSSL_DEBUG
#	tcsh:	setenv MATRIXSSL_DEBUG 1
#			unsetenv MATRIXSSL_DEBUG
#
ifdef MATRIXSSL_DEBUG
DFLAGS  = -g -Wall -DDEBUG
else
#DFLAGS	= -Os
DFLAGS	= -O3
endif

ifdef MATRIXSSL_PROFILE
DFLAGS	+= -g -pg
endif

#
#   Override variables for compilation on x86-64 (AMD64, Nacona, etc)
#
ifeq ($(shell uname -m),x86_64)
DFLAGS	+= -march=k8 -fPIC -DFP_64BIT -DTFM_X86_64 -DRDTSC
endif

#
#	Compile options
#
CFLAGS  = $(DFLAGS) -DLINUX
LDFLAGS = -lc

#
#   Override variables for compilation on Mac OS X (Darwin)
#
ifeq ($(shell uname),Darwin)
CC  = cc
SO  = .dylib
CFLAGS += -DOSX -isystem -I/usr/include
LDFLAGS =
endif

#
#   Override variables for compilation on uClinux (example only)
#
ifdef UCLINUX
CC		= /opt/brcm/hndtools-mipsel-uclibc/bin/mipsel-uclibc-gcc
STRIP	= /opt/brcm/hndtools-mipsel-uclibc/bin/mipsel-uclibc-strip
endif

all: compile

compile: $(REF_OBJS) $(REF_EXE) $(CLIENT_OBJS) $(CLIENT_EXE) 

#
# Manual dependencies
#
*.o: ../matrixSsl.h Makefile

$(REF_EXE):$(REF_OBJS)
	$(CC) -o $@ $^ $(LDFLAGS) $(LIBRARIES)

$(CLIENT_EXE):$(CLIENT_OBJS)
	$(CC) -o $@ $^ $(LDFLAGS) $(LIBRARIES)

#
#	Clean up all generated files
#
clean:
	rm -f $(REF_EXE) $(REF_OBJS) \
		$(CLIENT_EXE) $(CLIENT_OBJS) 
