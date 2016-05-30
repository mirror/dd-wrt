## src/classes/Makefile.am
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


EXTRA_DIST = \
	$(VM_JAVA_FILES_GNU_CLASSPATH) \
	$(VM_JAVA_FILES_GNU_CLASSPATH_ANNOTATIONS) \
	$(VM_JAVA_FILES_CLDC1_1)

CLEANFILES = vm.zip

VM_JAVA_FILES_GNU_CLASSPATH = \
	$(top_srcdir)/src/classes/gnuclasspath/gnu/classpath/VMStackWalker.java \
	$(top_srcdir)/src/classes/gnuclasspath/gnu/classpath/VMSystemProperties.java \
	$(top_srcdir)/src/classes/gnuclasspath/gnu/java/lang/VMCPStringBuilder.java \
	$(top_srcdir)/src/classes/gnuclasspath/gnu/java/lang/management/VMMemoryMXBeanImpl.java \
	$(top_srcdir)/src/classes/gnuclasspath/gnu/java/lang/management/VMRuntimeMXBeanImpl.java \
	$(top_srcdir)/src/classes/gnuclasspath/java/lang/VMClassLoader.java \
	$(top_srcdir)/src/classes/gnuclasspath/java/lang/VMString.java \
	$(top_srcdir)/src/classes/gnuclasspath/java/lang/VMThread.java \
	$(top_srcdir)/src/classes/gnuclasspath/java/lang/reflect/VMConstructor.java \
	$(top_srcdir)/src/classes/gnuclasspath/java/lang/reflect/VMField.java \
	$(top_srcdir)/src/classes/gnuclasspath/java/lang/reflect/VMMethod.java \
	$(top_srcdir)/src/classes/gnuclasspath/java/security/VMAccessController.java \
	$(top_srcdir)/src/classes/gnuclasspath/sun/misc/Unsafe.java

# Remove these files when a new GNU Classpath release (> 0.97.1) is
# available.

VM_JAVA_FILES_GNU_CLASSPATH += \
	$(top_srcdir)/src/classes/gnuclasspath/java/lang/reflect/Constructor.java \
	$(top_srcdir)/src/classes/gnuclasspath/java/lang/reflect/Field.java \
	$(top_srcdir)/src/classes/gnuclasspath/java/lang/reflect/Method.java \
	$(top_srcdir)/src/classes/gnuclasspath/java/lang/reflect/Modifier.java \
	$(top_srcdir)/src/classes/gnuclasspath/gnu/java/lang/CPStringBuilder.java

VM_JAVA_FILES_GNU_CLASSPATH_ANNOTATIONS = \
	$(top_srcdir)/src/classes/gnuclasspath/sun/reflect/ConstantPool.java \
	$(top_srcdir)/src/classes/gnuclasspath/sun/reflect/annotation/ExceptionProxy.java \
	$(top_srcdir)/src/classes/gnuclasspath/sun/reflect/annotation/EnumConstantNotPresentExceptionProxy.java \
	$(top_srcdir)/src/classes/gnuclasspath/sun/reflect/annotation/TypeNotPresentExceptionProxy.java \
	$(top_srcdir)/src/classes/gnuclasspath/sun/reflect/annotation/AnnotationTypeMismatchExceptionProxy.java \
	$(top_srcdir)/src/classes/gnuclasspath/sun/reflect/annotation/AnnotationType.java \
	$(top_srcdir)/src/classes/gnuclasspath/sun/reflect/annotation/AnnotationParser.java

VM_JAVA_FILES_CLDC1_1 = \
	$(top_srcdir)/src/classes/cldc1.1/com/sun/cldchi/jvm/FileDescriptor.java

if WITH_JAVA_RUNTIME_LIBRARY_GNU_CLASSPATH
VM_JAVA_FILES = \
	$(VM_JAVA_FILES_GNU_CLASSPATH)

if ENABLE_ANNOTATIONS
VM_JAVA_FILES += \
	$(VM_JAVA_FILES_GNU_CLASSPATH_ANNOTATIONS)
endif

if ENABLE_ZLIB
pkgdata_DATA = vm.zip
else
noinst_DATA = nozip
endif
endif

if WITH_JAVA_RUNTIME_LIBRARY_CLDC1_1
VM_JAVA_FILES = \
	$(VM_JAVA_FILES_CLDC1_1)

if ENABLE_ZLIB
pkgdata_DATA = vm.zip
else
noinst_DATA = nozip
endif
endif

if ENABLE_ZLIB
VM_ZIP = ../vm.zip

vm.zip: $(VM_JAVA_FILES)
	$(mkdir_p) classes
	$(JAVAC) -bootclasspath $(BOOTCLASSPATH) -source 1.5 -target 1.5 -d classes $(VM_JAVA_FILES)
	@if test "$(JAR)" = "zip" -o "$(JAR)" = "zip.exe"; then \
	    cd classes && $(JAR) -r -D $(VM_ZIP) .; \
	else \
	    cd classes && $(JAR) cvf $(VM_ZIP) .; \
	fi
else
nozip: $(VM_JAVA_FILES)
	$(mkdir_p) classes
	$(JAVAC) -bootclasspath $(BOOTCLASSPATH) -source 1.5 -target 1.5 -d classes $(VM_JAVA_FILES)

install-data-local: nozip
	$(mkinstalldirs) $(DESTDIR)$(pkgdatadir)
	cp -R classes $(DESTDIR)$(pkgdatadir)

uninstall-local:
	rm -rf $(DESTDIR)$(pkgdatadir)/classes

endif

clean-local:
	-rm -rf classes


## Local variables:
## mode: Makefile
## indent-tabs-mode: t
## c-basic-offset: 4
## tab-width: 8
## compile-command: "automake --add-missing"
## End:
