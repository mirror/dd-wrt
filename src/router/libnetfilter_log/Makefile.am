AM_CPPFLAGS = -I$(top_srcdir)/include
SUBDIRS	= include src utils
LINKOPTS = -lnfnetlink

ACLOCAL_AMFLAGS = -I m4

EXTRA_DIST = $(man_MANS) Make_global.am

man_MANS = #nfnetlink_log.3 nfnetlink_log.7

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libnetfilter_log.pc
