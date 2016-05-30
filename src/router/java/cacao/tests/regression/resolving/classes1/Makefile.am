## tests/regression/resolving/classes1/Makefile.am
##
## Copyright (C) 1996-2013
## CACAOVM - Verein zur Foerderung der freien virtuellen Maschine CACAO
##
## This file is part of CACAO.
##
## This program is free software; you can redistribute it and/or
## modify it under the terms of the GNU General Public License as
## published by the Free Software Foundation; either version 2, or (at
## your option) any later version.
##
## This program is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
## General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program; if not, write to the Free Software
## Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
## 02110-1301, USA.


SOURCE_FILES = \
	$(srcdir)/BarPassFoo.java \
	$(srcdir)/BarUseFoo.java \
	$(srcdir)/Foo.java

CLASS_FILES = \
	BarPassFoo.class \
	BarUseFoo.class \
	Foo.class

EXTRA_DIST = $(SOURCE_FILES)

CLEANFILES = \
	*.class

check: $(CLASS_FILES)

$(CLASS_FILES): $(SOURCE_FILES)
	$(JAVAC) -source 1.5 -target 1.5 -d . $(SOURCE_FILES)

