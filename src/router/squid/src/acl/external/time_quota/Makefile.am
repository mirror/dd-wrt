## Copyright (C) 1996-2024 The Squid Software Foundation and contributors
##
## Squid software is distributed under GPLv2+ license and includes
## contributions from numerous individuals and organizations.
## Please see the COPYING and CONTRIBUTORS files for details.
##

include $(top_srcdir)/src/Common.am

libexec_PROGRAMS= ext_time_quota_acl
man_MANS= ext_time_quota_acl.8

DEFS += -DDEFAULT_QUOTA_DB=\"$(localstatedir)/ext_time_quota.db\"

ext_time_quota_acl_SOURCES = \
	ext_time_quota_acl.cc
ext_time_quota_acl_LDADD = \
	$(COMPAT_LIB) \
	$(LIBTDB_LIBS)

EXTRA_DIST= ext_time_quota_acl.8 required.m4
