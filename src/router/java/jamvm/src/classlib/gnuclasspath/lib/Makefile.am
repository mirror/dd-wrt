##
## Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008
## Robert Lougher <rob@jamvm.org.uk>.
##
## This file is part of JamVM.
##
## This program is free software; you can redistribute it and/or
## modify it under the terms of the GNU General Public License
## as published by the Free Software Foundation; either version 2,
## or (at your option) any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program; if not, write to the Free Software
## Foundation, 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
##

CP_LIB_DIR = ${with_classpath_install_dir}/share/classpath
GLIBJ_ZIP  = ${CP_LIB_DIR}/glibj.zip

SUBDIRS = jamvm java gnu sun
EXTRA_DIST = classes.zip README

noinst_DATA = inst_classes.zip

JAVA_FILES = $(srcdir)/jamvm/java/lang/JarLauncher.java \
             $(srcdir)/jamvm/java/lang/VMClassLoaderData.java \
             $(srcdir)/jamvm/ThreadInfoHelper.java \
             $(srcdir)/java/lang/VMRuntime.java \
             $(srcdir)/java/lang/VMClass.java \
             $(srcdir)/java/lang/VMClassLoader.java \
             $(srcdir)/java/lang/VMString.java \
             $(srcdir)/java/lang/VMThread.java \
             $(srcdir)/java/lang/VMThrowable.java \
             $(srcdir)/java/lang/reflect/VMConstructor.java \
             $(srcdir)/java/lang/reflect/VMField.java \
             $(srcdir)/java/lang/reflect/VMMethod.java \
             $(srcdir)/java/security/VMAccessController.java \
             $(srcdir)/gnu/classpath/VMSystemProperties.java \
             $(srcdir)/sun/reflect/annotation/AnnotationInvocationHandler.java \
             $(srcdir)/gnu/classpath/VMStackWalker.java


inst_classes.zip: classes.zip
	ln -fs $< $@

classes.zip: $(JAVA_FILES)
	-mkdir classes
	$(JAVAC) -bootclasspath ${GLIBJ_ZIP} -d classes $(JAVA_FILES)
	cd classes && zip -r ../classes.zip .
	rm -rf classes

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(pkgdatadir)
	@use_zip_yes@ cp -p inst_classes.zip $(DESTDIR)$(pkgdatadir)/classes.zip
	@use_zip_no@ unzip -o inst_classes.zip -d $(DESTDIR)$(pkgdatadir)/classes
	@use_zip_yes@ $(mkinstalldirs) $(DESTDIR)$(libdir)
	@use_zip_yes@ ln -fs $(GLIBJ_ZIP) $(DESTDIR)$(libdir)/rt.jar

uninstall-local:
	rm -rf $(DESTDIR)$(pkgdatadir)
	rm $(DESTDIR)$(libdir)/rt.jar

DISTCLEANFILES = inst_classes.zip
