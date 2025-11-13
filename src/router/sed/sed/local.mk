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

bin_PROGRAMS += sed/sed

localedir = $(datadir)/locale

sed_sed_SOURCES =	\
  sed/compile.c		\
  sed/debug.c		\
  sed/execute.c		\
  sed/mbcs.c		\
  sed/regexp.c		\
  sed/sed.c		\
  sed/utils.c

noinst_HEADERS +=	\
  sed/sed.h		\
  sed/utils.h

sed_sed_CPPFLAGS = $(AM_CPPFLAGS) -DLOCALEDIR=\"$(localedir)\"
sed_sed_CFLAGS = $(AM_CFLAGS) $(WARN_CFLAGS) $(WERROR_CFLAGS)
sed_sed_LDADD = sed/libver.a lib/libsed.a $(INTLLIBS) $(LIB_ACL) $(LIB_SELINUX)
sed_sed_DEPENDENCIES = lib/libsed.a sed/libver.a

$(sed_sed_OBJECTS): $(BUILT_SOURCES)

BUILT_SOURCES += sed/version.c
DISTCLEANFILES += sed/version.c
sed/version.c: Makefile
	$(AM_V_GEN)rm -f $@
	$(AM_V_at)printf '#include <config.h>\n' > $@t
	$(AM_V_at)printf 'char const *Version = "$(PACKAGE_VERSION)";\n' >> $@t
	$(AM_V_at)chmod a-w $@t
	$(AM_V_at)mv $@t $@

BUILT_SOURCES += sed/version.h
DISTCLEANFILES += sed/version.h
sed/version.h: Makefile
	$(AM_V_GEN)rm -f $@
	$(AM_V_at)printf 'extern char const *Version;\n' > $@t
	$(AM_V_at)chmod a-w $@t
	$(AM_V_at)mv $@t $@

noinst_LIBRARIES += sed/libver.a
nodist_sed_libver_a_SOURCES = sed/version.c sed/version.h
