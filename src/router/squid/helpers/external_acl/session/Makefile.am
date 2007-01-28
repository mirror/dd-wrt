#
#  Makefile for the Squid LDAP authentication helper
#
#  $Id: Makefile.am,v 1.1 2006/05/22 20:43:38 hno Exp $
#
#  Uncomment and customize the following to suit your needs:
#

libexec_PROGRAMS 		= squid_session
man_MANS 			= squid_session.8
EXTRA_DIST			= squid_session.8
squid_session_SOURCES		= squid_session.c

LDADD = $(LIB_DB) 

INCLUDES	= -I. -I$(top_builddir)/include -I$(top_srcdir)/include
