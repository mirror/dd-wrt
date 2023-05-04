## $Id$
AUTOMAKE_OPTIONS=foreign no-dependencies
SUBDIRS = src doc etc templates rpm m4 preproc_rules tools

INCLUDES = @INCLUDES@

EXTRA_DIST = ChangeLog snort.8 LICENSE verstuff.pl RELEASE.NOTES snort.pc.in VERSION

man_MANS = snort.8

DISTCLEANFILES=stamp-h.in cflags.out cppflags.out

dist-hook:
# always ensure that the win32 build is in sync
	perl $(distdir)/verstuff.pl $(distdir)
# work around a horrible doc/Makefile.am
	rm -rf $(distdir)/doc/signatures/CVS

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = snort.pc
