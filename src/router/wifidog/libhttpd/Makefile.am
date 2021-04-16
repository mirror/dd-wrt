#
# $Id$
#

noinst_LIBRARIES = libhttpd.a

libhttpd_a_SOURCES = protocol.c \
	api.c \
	version.c \
	ip_acl.c

noinst_HEADERS = httpd_priv.h

pkginclude_HEADERS = httpd.h

EXTRA_DIST = README

AM_CPPFLAGS = \
	-Wall \
	-Wextra \
	-Wno-unused-parameter
