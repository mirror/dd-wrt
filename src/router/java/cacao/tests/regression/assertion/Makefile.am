## tests/regression/assertion/Makefile.am
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


JAVA     = $(top_builddir)/src/cacao/cacao
JAVACMD  = $(JAVA) -Xbootclasspath:$(BOOTCLASSPATH)
JAVACCMD = $(JAVAC) -bootclasspath $(BOOTCLASSPATH) -source 1.5 -target 1.5

SOURCE_FILES = \
	$(srcdir)/testassertions.java \
	$(srcdir)/packagetest/testassertions.java

EXTRA_DIST = \
	$(SOURCE_FILES) \
	Test.sh \
	\
	enabled.output \
	disabled.output

CLEANFILES = \
	*.class \
	*.thisoutput \
	packagetest/*.class

ASSERTION_TESTS = \
	EA_TEST1 \
	EA_TEST2 \
	EA_PACKAGE_TEST1 \
	EA_PACKAGE_TEST2 \
	EA_PACKAGE_TEST3 \
	EA_PACKAGE_TEST4 \
	EA_PACKAGE_TEST5 \
	DA_PACKAGE_TEST1 \
	DA_PACKAGE_TEST2 \
	DA_PACKAGE_TEST3 \
	DA_PACKAGE_TEST4 \
	DA_PACKAGE_TEST5 \
	EA_CLASS_TEST1 \
	EA_CLASS_TEST2 \
	EA_CLASS_TEST3 \
	EA_CLASS_TEST4 \
	EA_CLASS_TEST5 \
	DA_CLASS_TEST1 \
	DA_CLASS_TEST2 \
	DA_CLASS_TEST3 \
	DA_CLASS_TEST4 \
	DA_CLASS_TEST5 \
	ESA_TEST1 \
	ESA_TEST2

check: build run

build:
	$(JAVACCMD) -d . $(SOURCE_FILES)

run: $(ASSERTION_TESTS)

EA_TEST1:
	@LD_LIBRARY_PATH=$(top_builddir)/src/cacao/.libs $(SHELL) $(srcdir)/Test.sh "$(JAVACMD) -ea" testassertions eatest1 enabled $(srcdir)

EA_TEST2:
	@LD_LIBRARY_PATH=$(top_builddir)/src/cacao/.libs $(SHELL) $(srcdir)/Test.sh "$(JAVACMD) -enableassertions" testassertions eatest2 enabled $(srcdir)

EA_PACKAGE_TEST1:
	@LD_LIBRARY_PATH=$(top_builddir)/src/cacao/.libs $(SHELL) $(srcdir)/Test.sh "$(JAVACMD) -da -ea:packagetest..." packagetest.testassertions eapackagetest1 enabled $(srcdir)

EA_PACKAGE_TEST2:
	@LD_LIBRARY_PATH=$(top_builddir)/src/cacao/.libs $(SHELL) $(srcdir)/Test.sh "$(JAVACMD) -disableassertions -ea:packagetest..." packagetest.testassertions eapackagetest2 enabled $(srcdir)

EA_PACKAGE_TEST3:
	@LD_LIBRARY_PATH=$(top_builddir)/src/cacao/.libs $(SHELL) $(srcdir)/Test.sh "$(JAVACMD) -disableassertions -enableassertions:packagetest..." packagetest.testassertions eapackagetest3 enabled $(srcdir)

EA_PACKAGE_TEST4:
	@LD_LIBRARY_PATH=$(top_builddir)/src/cacao/.libs $(SHELL) $(srcdir)/Test.sh "$(JAVACMD) -enableassertions:packagetest... -disableassertions " packagetest.testassertions eapackagetest4 enabled $(srcdir)

EA_PACKAGE_TEST5:
	@LD_LIBRARY_PATH=$(top_builddir)/src/cacao/.libs $(SHELL) $(srcdir)/Test.sh "$(JAVACMD) -da -enableassertions:packagetest..." packagetest.testassertions eapackagetest5 enabled $(srcdir)

EA_CLASS_TEST1:
	@LD_LIBRARY_PATH=$(top_builddir)/src/cacao/.libs $(SHELL) $(srcdir)/Test.sh "$(JAVACMD) -da -ea:testassertions" testassertions eaclasstest1 enabled $(srcdir)

EA_CLASS_TEST2:
	@LD_LIBRARY_PATH=$(top_builddir)/src/cacao/.libs $(SHELL) $(srcdir)/Test.sh "$(JAVACMD) -disableassertions -ea:testassertions" testassertions eaclasstest2 enabled $(srcdir)

EA_CLASS_TEST3:
	@LD_LIBRARY_PATH=$(top_builddir)/src/cacao/.libs $(SHELL) $(srcdir)/Test.sh "$(JAVACMD) -disableassertions -enableassertions:testassertions" testassertions eaclasstest3 enabled $(srcdir)

EA_CLASS_TEST4:
	@LD_LIBRARY_PATH=$(top_builddir)/src/cacao/.libs $(SHELL) $(srcdir)/Test.sh "$(JAVACMD) -enableassertions:testassertions -disableassertions " testassertions eaclasstest4 enabled $(srcdir)

EA_CLASS_TEST5:
	@LD_LIBRARY_PATH=$(top_builddir)/src/cacao/.libs $(SHELL) $(srcdir)/Test.sh "$(JAVACMD) -da -enableassertions:testassertions" testassertions eaclasstest5 enabled $(srcdir)

DA_PACKAGE_TEST1:
	@LD_LIBRARY_PATH=$(top_builddir)/src/cacao/.libs $(SHELL) $(srcdir)/Test.sh "$(JAVACMD) -da:packagetest... -ea" packagetest.testassertions dapackagetest1 disabled $(srcdir)

DA_PACKAGE_TEST2:
	@LD_LIBRARY_PATH=$(top_builddir)/src/cacao/.libs $(SHELL) $(srcdir)/Test.sh "$(JAVACMD) -disableassertions:packagetest... -ea" packagetest.testassertions dapackagetest2 disabled $(srcdir)

DA_PACKAGE_TEST3:
	@LD_LIBRARY_PATH=$(top_builddir)/src/cacao/.libs $(SHELL) $(srcdir)/Test.sh "$(JAVACMD) -disableassertions:packagetest... -enableassertions" packagetest.testassertions dapackagetest3 disabled $(srcdir)

DA_PACKAGE_TEST4:
	@LD_LIBRARY_PATH=$(top_builddir)/src/cacao/.libs $(SHELL) $(srcdir)/Test.sh "$(JAVACMD) -enableassertions -disableassertions:packagetest... " packagetest.testassertions dapackagetest4 disabled $(srcdir)

DA_PACKAGE_TEST5:
	@LD_LIBRARY_PATH=$(top_builddir)/src/cacao/.libs $(SHELL) $(srcdir)/Test.sh "$(JAVACMD) -da:packagetest... -enableassertions" packagetest.testassertions dapackagetest5 disabled $(srcdir)

DA_CLASS_TEST1:
	@LD_LIBRARY_PATH=$(top_builddir)/src/cacao/.libs $(SHELL) $(srcdir)/Test.sh "$(JAVACMD) -ea -da:testassertions" testassertions daclasstest1 disabled $(srcdir)

DA_CLASS_TEST2:
	@LD_LIBRARY_PATH=$(top_builddir)/src/cacao/.libs $(SHELL) $(srcdir)/Test.sh "$(JAVACMD) -enableassertions -da:testassertions" testassertions daclasstest2 disabled $(srcdir)

DA_CLASS_TEST3:
	@LD_LIBRARY_PATH=$(top_builddir)/src/cacao/.libs $(SHELL) $(srcdir)/Test.sh "$(JAVACMD) -enableassertions -disableassertions:testassertions" testassertions daclasstest3 disabled $(srcdir)

DA_CLASS_TEST4:
	@LD_LIBRARY_PATH=$(top_builddir)/src/cacao/.libs $(SHELL) $(srcdir)/Test.sh "$(JAVACMD) -disableassertions:testassertions -enableassertions " testassertions daclasstest4 disabled $(srcdir)

DA_CLASS_TEST5:
	@LD_LIBRARY_PATH=$(top_builddir)/src/cacao/.libs $(SHELL) $(srcdir)/Test.sh "$(JAVACMD) -ea -disableassertions:testassertions" testassertions daclasstest5 disabled $(srcdir)

ESA_TEST1:
	@LD_LIBRARY_PATH=$(top_builddir)/src/cacao/.libs $(SHELL) $(srcdir)/Test.sh "$(JAVACMD) -esa" testassertions esatest1 disabled $(srcdir)

ESA_TEST2:
	@LD_LIBRARY_PATH=$(top_builddir)/src/cacao/.libs $(SHELL) $(srcdir)/Test.sh "$(JAVACMD) -enablesystemassertions" testassertions esatest2 disabled $(srcdir)


## Local variables:
## mode: Makefile
## indent-tabs-mode: t
## c-basic-offset: 4
## tab-width: 8
## compile-command: "automake --add-missing"
## End:
