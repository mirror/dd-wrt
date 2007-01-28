## Process this file with automake to produce Makefile.in
#
#  $Id: Makefile.am,v 1.9 2006/09/08 19:41:24 serassio Exp $
#

if NEED_OWN_SNPRINTF
SNPRINTFSOURCE=snprintf.c
else
SNPRINTFSOURCE=
endif
if NEED_OWN_STRSEP
STRSEPSOURCE=strsep.c
else
STRSEPSOURCE=
endif
if NEED_OWN_MD5
MD5SOURCE=md5.c
else
MD5SOURCE=
endif
if ENABLE_WIN32SPECIFIC
LIBSSPWIN32=libsspwin32.a
else
LIBSSPWIN32=
endif

if ENABLE_MINGW32SPECIFIC
WIN32SRC = win32lib.c
else
WIN32SRC=
endif

EXTRA_LIBRARIES = \
	libdlmalloc.a \
	libregex.a \
	libsspwin32.a
noinst_LIBRARIES = \
	@LIBDLMALLOC@ \
	libmiscutil.a \
	libntlmauth.a \
	@LIBREGEX@ \
	$(LIBSSPWIN32)
EXTRA_libmiscutil_a_SOURCES = \
	md5.c \
	snprintf.c \
	strsep.c \
	win32lib.c
libmiscutil_a_SOURCES = \
	Array.c \
	base64.c \
	getfullhostname.c \
	hash.c \
	heap.c \
	html_quote.c \
	iso3307.c \
	$(MD5SOURCE) \
	radix.c \
	rfc1035.c \
	rfc1123.c \
	rfc1738.c \
	rfc2617.c \
	safe_inet_addr.c \
	$(SNPRINTFSOURCE) \
	splay.c \
	Stack.c \
	$(STRSEPSOURCE) \
	stub_memaccount.c \
	util.c \
	uudecode.c \
	$(WIN32SRC)
libmiscutil_a_LIBADD = \
	@LIBOBJS@
# $(top_srcdir)/include/version.h should be a dependency
libregex_a_SOURCES = \
	GNUregex.c
libdlmalloc_a_SOURCES = \
	dlmalloc.c
libntlmauth_a_SOURCES = \
	ntlmauth.c
libntlmauth_a_LIBADD = \
	@LIBOBJS@
libsspwin32_a_SOURCES = \
	sspwin32.c

INCLUDES	= -I$(top_builddir)/include -I$(top_srcdir)/include
