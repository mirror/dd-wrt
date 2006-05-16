## Process this file with automake to create Makefile.in
#
# Copyright (C) 1996-2002 Markus F.X.J. Oberhumer
#

### LZO_DEBUG = -DLZO_DEBUG
### LZO_DEBUG = -DLZO_DEBUG -g -fno-omit-frame-pointer

INCLUDES = -I$(top_srcdir)/include
CFLAGS = @CFLAGS@ $(CFLAGS_O) $(CFLAGS_W) $(LZO_DEBUG)


noinst_PROGRAMS = ltest

ltest_SOURCES = ltest.c asm.h db.h wrap.h wrapmisc.h timer.h mygetopt.ch mygetopt.h
ltest_LDADD = $(liblzo) $(ltest_LIBS)
ltest_DEPENDENCIES = $(liblzo)


@liblzo@:
	cd $(top_builddir)/src && $(MAKE) all

src:
	cd $(top_builddir)/src && $(MAKE) all
	$(MAKE) all

