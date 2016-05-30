## src/tests/regression/native/Makefile.am
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


AM_CPPFLAGS += -I$(top_srcdir) -I$(top_srcdir)/src -I$(top_srcdir)/src/vm/jit/$(ARCH_DIR) -I$(top_srcdir)/src/vm/jit/$(ARCH_DIR)/$(OS_DIR) -I$(top_builddir)

JAVA     = $(top_builddir)/src/cacao/cacao
JAVACMD  = $(JAVA) -Xbootclasspath:$(BOOTCLASSPATH)
JAVACCMD = $(JAVAC) -source 1.5 -target 1.5 -bootclasspath $(BOOTCLASSPATH)

SOURCE_FILES =         \
	checkjni.java      \
	testarguments.java

EXTRA_DIST =              \
	$(SOURCE_FILES)       \
	checkjni.cpp          \
	testarguments.cpp     \
	                      \
	checkjni.output       \
	testarguments.output

CLEANFILES =       \
	*.class        \
	*.h            \
	*.so           \
	*.thisoutput   \
	*.this2output

NOTESTNAMES = \
	test

TESTNAMES =        \
	checkjni       \
	testarguments

check: $(TESTNAMES)

$(TESTNAMES) $(NOTESTNAMES):
	@$(JAVACCMD) -d . $(srcdir)/$@.java
	@$(CXX) -shared $(AM_CPPFLAGS) $(CPPFLAGS) $(CFLAGS) $(srcdir)/$@.cpp -o lib$@$(OS_DYNLIB) -fPIC
	@LD_LIBRARY_PATH=$(top_builddir)/src/cacao/.libs:. $(SHELL) $(srcdir)/../Test.sh "$(JAVACMD)" $@ $(srcdir)


## Local variables:
## mode: Makefile
## indent-tabs-mode: t
## c-basic-offset: 4
## tab-width: 8
## compile-command: "automake --add-missing"
## End:
