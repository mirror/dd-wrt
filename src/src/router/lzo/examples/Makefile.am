## Process this file with automake to create Makefile.in
#
# Copyright (C) 1996-2002 Markus F.X.J. Oberhumer
#

AUTOMAKE_OPTIONS = 1.6


INCLUDES = -I$(top_srcdir)/include
CFLAGS = @CFLAGS@ $(CFLAGS_O) $(CFLAGS_W)


noinst_PROGRAMS = dict lpack overlap precomp precomp2 simple

LDADD = @liblzo@

EXTRA_DIST = lutil.h


@liblzo@:
	cd $(top_builddir)/src && $(MAKE) all

