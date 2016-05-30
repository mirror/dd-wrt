## tests/regression/Makefile.am
##
## Copyright (C) 1996-2005, 2006, 2007, 2008
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


SUBDIRS = \
	base \
	assertion \
	bugzilla \
	jasmin \
	native \
	resolving

JAVA     = $(top_builddir)/src/cacao/cacao
JAVACMD  = $(JAVA) -Xbootclasspath:$(BOOTCLASSPATH)
JAVACCMD = $(JAVAC) -source 1.5 -target 1.5 -nowarn -bootclasspath $(BOOTCLASSPATH)

SOURCE_FILES = \
	$(srcdir)/jctest.java \
	$(srcdir)/fptest.java \
	$(srcdir)/fp.java \
	$(srcdir)/extest.java \
	$(srcdir)/LoadDisplacementOverflow.java \
	$(srcdir)/FieldDisplacementOverflow.java \
	$(srcdir)/StackDisplacementOverflow.java \
	$(srcdir)/MinimalClassReflection.java \
	$(srcdir)/TestAnnotations.java

EXTRA_DIST = \
	$(SOURCE_FILES) \
	Test.sh \
	\
	jctest.output \
	fptest.output.cp \
	fptest.output.ojdk \
	fp.output.cp \
	fp.output.ojdk \
	extest.2output.cp \
	extest.2output.ojdk \
	LoadDisplacementOverflow.output \
	FieldDisplacementOverflow.output \
	StackDisplacementOverflow.output \
	MinimalClassReflection.output \
	TestAnnotations.output

CLEANFILES = \
	*.class \
	*.thisoutput

OUTPUT_JAVA_TESTS = \
	jctest \
	fptest \
	fp \
	extest \
	\
	LoadDisplacementOverflow \
	FieldDisplacementOverflow \
	StackDisplacementOverflow \
	MinimalClassReflection \
	TestAnnotations

check: build run

build:
	$(JAVACCMD) -d . $(SOURCE_FILES)

run: $(OUTPUT_JAVA_TESTS)

$(OUTPUT_JAVA_TESTS):
	@LD_LIBRARY_PATH=$(top_builddir)/src/cacao/.libs $(SHELL) $(srcdir)/Test.sh "$(JAVACMD)" $@ $(srcdir)


## Local variables:
## mode: Makefile
## indent-tabs-mode: t
## c-basic-offset: 4
## tab-width: 8
## compile-command: "automake --add-missing"
## End:
