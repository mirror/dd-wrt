# $Id: Makefile,v 1.19 2012/08/21 17:24:07 nanard Exp $
# This Makefile is designed for use with GNU make
# libnatpmp
# (c) 2007-2018 Thomas Bernard
# http://miniupnp.free.fr/libnatpmp.html

OS = $(shell $(CC) -dumpmachine)
CC ?= gcc
ARCH = $(OS)
VERSION = $(shell cat VERSION)
INSTALL ?= $(shell which install)

ifneq (, $(findstring darwin, $(OS)))
JARSUFFIX=mac
LIBTOOL ?= $(shell which libtool)
else
ifneq (, $(findstring linux, $(OS)))
JARSUFFIX=linux
else
ifneq (, $(findstring mingw, $(OS))$(findstring cygwin, $(OS))$(findstring msys, $(OS)))
JARSUFFIX=win32
endif
endif
endif

# APIVERSION is used in soname
APIVERSION = 1
#LDFLAGS = -Wl,--no-undefined
CFLAGS ?= -Os
#CFLAGS = -g -O0
CFLAGS += -fPIC
CFLAGS += -Wall
CFLAGS += -Wextra
CFLAGS += -DENABLE_STRNATPMPERR

LIBOBJS = natpmp.o getgateway.o

OBJS = $(LIBOBJS) testgetgateway.o natpmpc.o natpmp-jni.o

STATICLIB = libnatpmp.a
ifneq (, $(findstring darwin, $(OS)))
  SHAREDLIB = libnatpmp.dylib
  JNISHAREDLIB = libjninatpmp.jnilib
  SONAME = $(basename $(SHAREDLIB)).$(APIVERSION).dylib
  CFLAGS := -DMACOSX -D_DARWIN_C_SOURCE $(CFLAGS) -I/System/Library/Frameworks/JavaVM.framework/Headers
  SONAMEFLAGS=-Wl,-install_name,$(JNISHAREDLIB) -framework JavaVM
else
ifneq (, $(findstring mingw, $(OS))$(findstring cygwin, $(OS))$(findstring msys, $(OS)))
  SHAREDLIB = natpmp.dll
  JNISHAREDLIB = jninatpmp.dll
  CC = i686-w64-mingw32-gcc
  LDLIBS += -lws2_32 -lIphlpapi
  LDFLAGS += -Wl,--no-undefined -Wl,--enable-runtime-pseudo-reloc
  #LDFLAGS += --Wl,kill-at
  LIBOBJS += wingettimeofday.o
else
  SHAREDLIB = libnatpmp.so
  JNISHAREDLIB = libjninatpmp.so
  SONAME = $(SHAREDLIB).$(APIVERSION)
  SONAMEFLAGS=-Wl,-soname,$(JNISHAREDLIB)
endif
endif

HEADERS = natpmp.h natpmp_declspec.h

EXECUTABLES = testgetgateway natpmpc-shared natpmpc-static

LIBDIR ?= lib

INSTALLPREFIX ?= $(PREFIX)/usr
INSTALLDIRINC = $(INSTALLPREFIX)/include

INSTALLDIRLIB = $(INSTALLPREFIX)/$(LIBDIR)
ifneq (, $(findstring x86_64, $(ARCH)))
INSTALLDIRLIB = $(INSTALLPREFIX)/lib64
endif

INSTALLDIRBIN = $(INSTALLPREFIX)/bin
PKGCONFIGDIR  = $(INSTALLDIRLIB)/pkgconfig

JAVA ?= java
JAVAC ?= javac
JAVAH ?= javah
JAVAPACKAGE = fr/free/miniupnp/libnatpmp
JAVACLASSES = $(JAVAPACKAGE)/NatPmp.class $(JAVAPACKAGE)/NatPmpResponse.class $(JAVAPACKAGE)/LibraryExtractor.class $(JAVAPACKAGE)/URLUtils.class
JNIHEADERS = fr_free_miniupnp_libnatpmp_NatPmp.h

.PHONY:	all clean depend install cleaninstall installpythonmodule

all: $(STATICLIB) $(SHAREDLIB) $(EXECUTABLES)

pythonmodule: $(STATICLIB) libnatpmpmodule.c setup.py
	MAKE=$(MAKE) python setup.py build
	touch $@

installpythonmodule: pythonmodule
	MAKE=$(MAKE) python setup.py install

clean:
	$(RM) $(OBJS) $(EXECUTABLES) $(STATICLIB) $(SHAREDLIB) $(JAVACLASSES) $(JNISHAREDLIB)
	$(RM) pythonmodule
	$(RM) natpmp.pc
	$(RM) -r build/ dist/ libraries/
	$(RM) JavaTest.class fr_free_miniupnp_libnatpmp_NatPmp.h

distclean: clean
	$(RM) *.jar out.errors.txt

depend:
	makedepend -f$(MAKEFILE_LIST) -Y $(OBJS:.o=.c) 2>/dev/null

install:	$(HEADERS) $(STATICLIB) $(SHAREDLIB) natpmpc-shared natpmp.pc
	$(INSTALL) -d $(INSTALLDIRINC)
	$(INSTALL) -m 644 $(HEADERS) $(INSTALLDIRINC)
	$(INSTALL) -d $(INSTALLDIRLIB)
	$(INSTALL) -m 644 $(STATICLIB) $(INSTALLDIRLIB)
	$(INSTALL) -m 644 $(SHAREDLIB) $(INSTALLDIRLIB)/$(SONAME)
	$(INSTALL) -d $(DESTDIR)$(PKGCONFIGDIR)
	$(INSTALL) -m 644 natpmp.pc $(DESTDIR)$(PKGCONFIGDIR)
	$(INSTALL) -d $(INSTALLDIRBIN)
	$(INSTALL) -m 755 natpmpc-shared $(INSTALLDIRBIN)/natpmpc
	ln -s -f $(SONAME) $(INSTALLDIRLIB)/$(SHAREDLIB)

$(JNIHEADERS): fr/free/miniupnp/libnatpmp/NatPmp.class
	$(JAVAH) -jni fr.free.miniupnp.libnatpmp.NatPmp

%.class: %.java
	$(JAVAC) -cp . $<

$(JNISHAREDLIB): $(JNIHEADERS) $(JAVACLASSES) $(LIBOBJS)
ifeq (,$(JAVA_HOME))
	@echo "Check your JAVA_HOME environement variable" && false
endif
ifneq (, $(findstring mingw, $(OS))$(findstring cygwin, $(OS))$(findstring msys, $(OS)))
	$(CC) -m32 -D_JNI_Implementation_ -Wl,--kill-at \
	-I"$(JAVA_HOME)/include" -I"$(JAVA_HOME)/include/win32" \
	natpmp-jni.c -shared \
	-o $(JNISHAREDLIB) -L. -lnatpmp -lws2_32 -lIphlpapi
else
	$(CC) $(CFLAGS) -c -I"$(JAVA_HOME)/include" natpmp-jni.c
ifneq (, $(findstring darwin, $(OS)))
	$(CC) $(LDFLAGS) -o $(JNISHAREDLIB) -dynamiclib $(SONAMEFLAGS) natpmp-jni.o -lc $(LIBOBJS)
else
	$(CC) $(LDFLAGS) -o $(JNISHAREDLIB) -shared $(SONAMEFLAGS) natpmp-jni.o -lc $(LIBOBJS)
endif
endif

jar: $(JNISHAREDLIB)
	find fr -name '*.class' -print > classes.list
	$(eval JNISHAREDLIBPATH := $(shell java fr.free.miniupnp.libnatpmp.LibraryExtractor))
	mkdir -p libraries/$(JNISHAREDLIBPATH)
	mv $(JNISHAREDLIB) libraries/$(JNISHAREDLIBPATH)/$(JNISHAREDLIB)
	$(RM) natpmp_$(JARSUFFIX).jar
	jar cf natpmp_$(JARSUFFIX).jar @classes.list libraries/$(JNISHAREDLIBPATH)/$(JNISHAREDLIB)
	$(RM) classes.list

jnitest: $(JNISHAREDLIB) JavaTest.class
	$(RM) libjninatpmp.so
	$(JAVA) -Djna.nosys=true -cp . JavaTest

mvn_install:
	mvn install:install-file -Dfile=java/natpmp_$(JARSUFFIX).jar \
	 -DgroupId=com.github \
	 -DartifactId=natpmp \
	 -Dversion=$(VERSION) \
	 -Dpackaging=jar \
	 -Dclassifier=$(JARSUFFIX) \
	 -DgeneratePom=true \
	 -DcreateChecksum=true

cleaninstall:
	$(RM) $(addprefix $(INSTALLDIRINC), $(HEADERS))
	$(RM) $(INSTALLDIRLIB)/$(SONAME)
	$(RM) $(INSTALLDIRLIB)/$(SHAREDLIB)
	$(RM) $(INSTALLDIRLIB)/$(STATICLIB)
	$(RM) $(INSTALLDIRLIB)/$(PKGCONFIGDIR)

natpmp.pc: VERSION
	$(RM) $@
	echo "prefix=$(INSTALLPREFIX)" >> $@
	echo "exec_prefix=\$${prefix}" >> $@
	echo "libdir=\$${exec_prefix}/$(LIBDIR)" >> $@
	echo "includedir=\$${prefix}/include" >> $@
	echo "" >> $@
	echo "Name: libnatpmp" >> $@
	echo "Description: NAT-PMP client library" >> $@
	echo "Version: $(VERSION)" >> $@
	echo "Libs: -L\$${libdir} -lnatpmp" >> $@
	echo "Cflags: -I\$${includedir}" >> $@

testgetgateway:	testgetgateway.o getgateway.o

natpmpc-static:	natpmpc.o $(STATICLIB)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

natpmpc-shared:	natpmpc.o $(SHAREDLIB)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(STATICLIB):	$(LIBOBJS)
ifneq (, $(findstring darwin, $(OS)))
	$(LIBTOOL) -static -o $@ $?
else
	$(AR) crs $@ $?
endif

$(SHAREDLIB):	$(LIBOBJS)
ifneq (, $(findstring darwin, $(OS)))
#	$(CC) -dynamiclib $(LDFLAGS) -Wl,-install_name,$(SONAME) -o $@ $^ $(LDLIBS)
	$(CC) -dynamiclib $(LDFLAGS) -Wl,-install_name,$(INSTALLDIRLIB)/$(SONAME) -o $@ $^ $(LDLIBS)
else
	$(CC) -shared $(LDFLAGS) -Wl,-soname,$(SONAME) -o $@ $^ $(LDLIBS)
endif


# DO NOT DELETE

natpmp.o: natpmp.h getgateway.h natpmp_declspec.h
getgateway.o: getgateway.h natpmp_declspec.h
testgetgateway.o: getgateway.h natpmp_declspec.h
natpmpc.o: natpmp.h
