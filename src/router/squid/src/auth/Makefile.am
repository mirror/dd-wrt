#  Makefile for authentication modules in the Squid Object Cache server
#
#  $Id: Makefile.am,v 1.4 2006/06/02 21:35:38 hno Exp $
#
AUTOMAKE_OPTIONS = subdir-objects

EXTRA_LIBRARIES	= libbasic.a libdigest.a libntlm.a libnegotiate.a
noinst_LIBRARIES	= @AUTH_LIBS@

libbasic_a_SOURCES	= basic/auth_basic.c basic/auth_basic.h
libdigest_a_SOURCES	= digest/auth_digest.c digest/auth_digest.h
libntlm_a_SOURCES	= ntlm/auth_ntlm.c ntlm/auth_ntlm.h
libnegotiate_a_SOURCES	= negotiate/auth_negotiate.c negotiate/auth_negotiate.h

INCLUDES      = -I. -I$(top_builddir)/include -I$(top_srcdir)/include \
	-I$(top_srcdir)/src
