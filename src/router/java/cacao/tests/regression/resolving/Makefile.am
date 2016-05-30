## tests/regression/resolving/Makefile.am
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


SUBDIRS = \
	classes1 \
	classes2 \
	classes3

JAVA     = $(top_builddir)/src/cacao/cacao
JAVACMD  = $(JAVA) -Xbootclasspath:$(BOOTCLASSPATH)
JAVACCMD = $(JAVAC) -source 1.5 -target 1.5 -bootclasspath $(BOOTCLASSPATH)

HARNESS_SOURCE_FILES =            \
	$(srcdir)/TestController.java \
	$(srcdir)/TestLoader.java     \
	$(srcdir)/ClassLibrary.java

HARNESS_CLASS_FILES =    \
	TestController.class \
	TestLoader.class     \
	ClassLibrary.class

TEST_SOURCE_FILES = \
	$(srcdir)/test_instance_subtype_violated.java \
	$(srcdir)/test_param_loading_constraint_violated_derived.java \
	$(srcdir)/test_param_loading_constraint_violated.java \
	$(srcdir)/test_param_subtype_violated.java \
	$(srcdir)/test_retval_loading_constraint_violated.java \
	$(srcdir)/test_return_subtype_ok.java \
	$(srcdir)/test_return_subtype_violated.java \
	$(srcdir)/test_simple_lazy_load.java

TEST_NAMES = \
	test_instance_subtype_violated \
	test_param_loading_constraint_violated_derived \
	test_param_loading_constraint_violated \
	test_param_subtype_violated \
	test_retval_loading_constraint_violated \
	test_return_subtype_ok \
	test_return_subtype_violated \
	test_simple_lazy_load

EXTRA_DIST = $(HARNESS_SOURCE_FILES) $(TEST_SOURCE_FILES)

CLEANFILES = \
	*.class

check: $(HARNESS_CLASS_FILES)
	@for t in $(TEST_NAMES) ; do echo "TEST $$t" ; if ! { $(JAVACCMD) -d . -classpath . $(srcdir)/$$t.java && LD_LIBRARY_PATH=$(top_builddir)/src/cacao/.libs $(JAVACMD) $$t ; }; then test -z "$(CHECK_NO_FAIL_FAST)" && exit 1 || echo FAIL; fi; done

$(HARNESS_CLASS_FILES): $(HARNESS_SOURCE_FILES)
	$(JAVACCMD) -d . $(HARNESS_SOURCE_FILES)
