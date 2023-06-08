# Copyright (C) 2002, 2004, 2008, 2014
#               Enrico Scholz <enrico.scholz@ensc.de>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; version 3 of the License.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see http://www.gnu.org/licenses/.

ACLOCAL_AMFLAGS		=  -I m4

WWW_HOST		=  janus
WWW_DIR			=  public_html/dhcp-fwd
WWW_FILES_ROOT		=  $(srcdir)/doc/index.html
WWW_FILES_FILES		=  $(distdir).tar.bz2 $(distdir).tar.bz2.asc $(srcdir)/README $(srcdir)/NEWS

man_MANS		=  doc/dhcp-fwd.1

SUBDIRS			=  . test
DIST_SUBDIRS		=  $(SUBDIRS)

confdir			=  $(sysconfdir)
cfg_filename		=  $(confdir)/dhcp-fwd.conf

logging_SRCS		=  src/logging.c

if WITH_LOGGING
pos_logging_SRCS	=  $(logging_SRCS)
neg_logging_SRCS	=
else
pos_logging_SRCS	=
neg_logging_SRCS	=  $(logging_SRCS)
endif

pos_SRCS		=  $(pos_logging_SRCS)
neg_SRCS		=  $(neg_logging_SRCS)

sbin_PROGRAMS		=  dhcp-fwd
EXTRA_dhcp_fwd_SOURCES	=  $(logging_SRCS)
dhcp_fwd_SOURCES	=  src/parser.c src/main.c src/cfg.c src/recvfromflags.c	\
			   src/output.c $(pos_SRCS)

AM_CPPFLAGS		=  -DCFG_FILENAME='"$(cfg_filename)"'

splint_HDRS		=  src/splint.h src/splint_compat.h

noinst_HEADERS		=  $(splint_HDRS) src/compat.h src/cfg.h		\
			   src/dhcp.h src/inet.h src/recvfromflags.h		\
			   src/util.h src/wrappers.h src/assertions.h		\
			   src/parser.h src/output.h src/logging.h		\
			   src/ulimit_codes.h

CLEANFILES		=  doc/dhcp-fwd.1 */*.gcov */*.gcda */*.gcno

EXTRA_DIST		=  dhcp-forwarder.spec Makefile.m4.am			\
			   doc/dhcp-fwd.man

%.1:			%.man
			$(MAKE) substvar VARS='sysconfdir sbindir PACKAGE VERSION' INFILE='$<' OUTFILE='$@'

$(PACKAGE).lcd:		Makefile.am splint.conf $(noinst_HEADERS)
			$(SPLINT) $(DEFAULT_INCLUDES) $(INCLUDES) -dump $@.tmp -f $(filter-out %Makefile.am,$^)
			-rm -f $@
			mv $@.tmp $@

splint:			splint.conf $(dhcp_fwd_SOURCES) $(neg_SRCS)
			$(SPLINT) $(DEFAULT_INCLUDES) $(INCLUDES) -f $^

www:			.www-timestamp

.www-timestamp:		sftp.cmds
			sftp -b $< $(WWW_HOST)
			touch $@

sftp.cmds:		$(WWW_FILES_ROOT) $(WWW_FILES_FILES)
			@echo 'chdir $(WWW_DIR)' >$@.tmp
			list="$(WWW_FILES_ROOT)"; for f in $$list; do			\
				test $$f -ot $@ || echo "put -P $$f";			\
			done >>$@.tmp

			@echo 'chdir files' >>$@.tmp
			list="$(WWW_FILES_FILES)"; for f in $$list; do			\
				test $$f -ot $@ || echo "put -P $$f";			\
			done >>$@.tmp
			mv -f $@.tmp $@

.PHONY:			splint

include $(top_srcdir)/m4/changelog.am
include $(top_srcdir)/m4/diet.am
include $(top_srcdir)/m4/gpgsig.am
include $(top_srcdir)/m4/substrule.am
include $(top_srcdir)/contrib/Makefile-files
