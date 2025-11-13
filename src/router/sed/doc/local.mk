# Copyright (C) 2016-2022 Free Software Foundation, Inc.

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

info_TEXINFOS = doc/sed.texi
doc_sed_TEXINFOS = doc/config.texi doc/version.texi doc/fdl.texi
dist_man_MANS = doc/sed.1
dist_noinst_DATA = doc/sed.x doc/sed-dummy.1
HELP2MAN = $(top_srcdir)/build-aux/help2man
SEDBIN = sed/sed
EXTRA_DIST += doc/dummy-man

AM_MAKEINFOHTMLFLAGS = --no-split

## Use the distributed man pages if cross compiling or lack perl
if CROSS_COMPILING
run_help2man = $(SHELL) $(srcdir)/doc/dummy-man
else
## Graceful degradation for systems lacking perl.
if HAVE_PERL
if BOLD_MAN_REFS
help2man_OPTS=--bold-refs
endif
run_help2man = $(PERL) -- $(HELP2MAN) $(help2man_OPTS)
else
run_help2man = $(SHELL) $(srcdir)/doc/dummy-man
endif
endif

doc/sed.1: sed/sed$(EXEEXT) .version $(srcdir)/doc/sed.x
	$(AM_V_GEN)$(MKDIR_P) doc
	$(AM_V_at)rm -rf $@ $@-t
	$(AM_V_at)$(run_help2man)					\
	    --info-page=sed						\
	    --include $(srcdir)/doc/sed.x				\
	    --output=$@-t						\
	    $(SEDBIN)							\
	  && chmod a-w $@-t						\
	  && mv $@-t $@
