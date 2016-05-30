## tests/regression/bugzilla/Makefile.am
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


JAVA     = $(top_builddir)/src/cacao/cacao
JAVACMD  = $(JAVA) -Xbootclasspath:$(BOOTCLASSPATH)
JAVACCMD = $(JAVAC) -source 1.5 -target 1.5 -nowarn -bootclasspath $(BOOTCLASSPATH)
JAVAENV  = LD_LIBRARY_PATH=$(top_builddir)/src/cacao/.libs

EXTRA_DIST = \
	$(srcdir)/*.java $(srcdir)/*.j $(srcdir)/boot-manifest.txt

CLEANFILES = \
	*.class boottest.jar

check: build run

build:
	$(JAVAENV) $(JAVACMD) -cp $(JASMIN_JAR) jasmin.Main $(srcdir)/*.j
	$(JAVAENV) $(JAVACCMD) -classpath $(JUNIT_JAR):$(builddir) -d . $(srcdir)/*.java
	$(JAR) cmf $(srcdir)/boot-manifest.txt boottest.jar boottest/

run:
	$(JAVAENV) $(JAVACMD) -Dcacao.test.javacmd="$(JAVACMD)" -classpath $(JUNIT_JAR):. org.junit.runner.JUnitCore All

clean-local:
	rm -rf boottest

## Local variables:
## mode: Makefile
## indent-tabs-mode: t
## c-basic-offset: 4
## tab-width: 8
## compile-command: "automake --add-missing"
## End:
