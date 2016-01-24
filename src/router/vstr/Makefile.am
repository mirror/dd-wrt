# See if we can cast the automake spell. -*-Makefile-*-
#

AUTOMAKE_OPTIONS = no-dependencies 1.4

SUBDIRS = src include tst

if USE_COMPILE_EXAMPLES
SUBDIRS += examples
endif

# uses examples to generate docs.
SUBDIRS += Documentation

pkgconfigdir   = $(libdir)/pkgconfig
pkgconfig_DATA = vstr.pc

$(pkgconfig_DATA): config.status

