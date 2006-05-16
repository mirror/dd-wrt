## Process this file with automake to create Makefile.in
#
# Copyright (C) 1996-2002 Markus F.X.J. Oberhumer
#

AUTOMAKE_OPTIONS = 1.6


INCLUDES = -I$(top_srcdir)/include -I$(top_srcdir)/src
CFLAGS = @CFLAGS@ $(CFLAGS_O) $(CFLAGS_W) $(LZO_CFLAGS)


noinst_PROGRAMS = align chksum promote sizes

align_LDADD = @liblzo@
chksum_LDADD = @liblzo@


@liblzo@:
	cd $(top_builddir)/src && $(MAKE) all

