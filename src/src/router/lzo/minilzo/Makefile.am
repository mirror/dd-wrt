## Process this file with automake to create Makefile.in
#
# Copyright (C) 1996-2002 Markus F.X.J. Oberhumer
#

AUTOMAKE_OPTIONS = 1.6


# /***********************************************************************
# // compiler-architecture-CPU specific CFLAGS
# ************************************************************************/

A_CFLAGS = @CFLAGS@
A_CFLAGS_O = @CFLAGS_O@
A_CFLAGS_W = @CFLAGS_W@

LZO_CFLAGS_GCC   =
LZO_CFLAGS_GCC_O =
LZO_CFLAGS_GCC_W = \
    -Wpointer-arith -Wstrict-prototypes -Wmissing-prototypes \
    -Wno-uninitialized

LZO_CFLAGS_GCC_i386   =
LZO_CFLAGS_GCC_i386_O = @CFLAGS_GCC_OMIT_FRAME_POINTER@ -fforce-addr
LZO_CFLAGS_GCC_i386_W =

LZO_CFLAGS_GCC_i386_i386_O = -m386
LZO_CFLAGS_GCC_i386_i486_O = -m486
LZO_CFLAGS_GCC_i386_i586_O =
LZO_CFLAGS_GCC_i386_i686_O =

L_CFLAGS = $(LZO_CFLAGS_@M_CC@) $(LZO_CFLAGS_@M_CC@_@M_ARCH@) $(LZO_CFLAGS_@M_CC@_@M_ARCH@_@M_CPU@)
L_CFLAGS_O = $(LZO_CFLAGS_@M_CC@_O) $(LZO_CFLAGS_@M_CC@_@M_ARCH@_O) $(LZO_CFLAGS_@M_CC@_@M_ARCH@_@M_CPU@_O)
L_CFLAGS_W = $(LZO_CFLAGS_@M_CC@_W) $(LZO_CFLAGS_@M_CC@_@M_ARCH@_W) $(LZO_CFLAGS_@M_CC@_@M_ARCH@_@M_CPU@_W)


# /***********************************************************************
# //
# ************************************************************************/

### LZO_DEBUG = -DLZO_DEBUG

INCLUDES = -I$(top_srcdir)/include

CPPFLAGS = @CPPFLAGS@ -DMINILZO_HAVE_CONFIG_H
CFLAGS = $(A_CFLAGS) $(L_CFLAGS) $(A_CFLAGS_O) $(L_CFLAGS_O) $(A_CFLAGS_W) $(L_CFLAGS_W) $(LZO_CFLAGS) $(LZO_DEBUG)


noinst_PROGRAMS = testmini

minilzo_sources = \
    README.LZO Makefile.lzo \
    minilzo.h minilzo.c

testmini_SOURCES = testmini.c $(minilzo_sources)


@MAINT@@GMAKE_TRUE@-include $(top_srcdir)/maint/make/minilzo.mk

