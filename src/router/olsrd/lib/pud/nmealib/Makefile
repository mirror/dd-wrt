include ../Makefile.inc
OLSRD_PLUGIN = false

TOPDIR = ../../..
include $(TOPDIR)/Makefile.inc


include Makefile.inc

#
# Settings
#

LIBNAME = libnmea.so
LIBNAMESTATIC = libnmea.a

DESTDIR ?=
USRDIR ?= $(DESTDIR)/usr
INCLUDEDIR ?= $(DESTDIR)/usr/include
LIBDIR ?= $(USRDIR)/lib


MODULES = context conversions generate generator gmath info parse parser sentence tok
OBJ = $(MODULES:%=build/%.o)

LIBRARIES = -lm
INCLUDES = -I ./include


#
# Targets
#

all: default_target

default_target: all-before lib/$(LIBNAMESTATIC) lib/$(LIBNAME)

remake: clean all

lib/$(LIBNAMESTATIC): $(OBJ)
ifeq ($(VERBOSE),0)
	@echo "[AR] $@"
endif
	$(MAKECMDPREFIX)ar rcs "$@" $(OBJ)

lib/$(LIBNAME): $(OBJ)
ifeq ($(VERBOSE),0)
	@echo "[LD] $@"
endif
	$(MAKECMDPREFIX)$(CC) $(LDFLAGS) -Wl,-soname=$(LIBNAME) -o "$@" $(LIBRARIES) $(OBJ)

build/%.o: src/%.c Makefile Makefile.inc
ifeq ($(VERBOSE),0)
	@echo "[CC] $<"
endif
	$(MAKECMDPREFIX)$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@


#
# Phony Targets
#

.PHONY: all default_target all-before clean doc install install-headers uninstall uninstall-headers

all-before:
	$(MAKECMDPREFIX)mkdir -p build lib

clean:
	$(MAKECMDPREFIX)$(MAKE) -C doc clean
	$(MAKECMDPREFIX)rm -frv build lib

doc:
	$(MAKECMDPREFIX)$(MAKE) -C doc all

doc-clean:
	$(MAKECMDPREFIX)$(MAKE) -C doc clean

install: all
	$(MAKECMDPREFIX)mkdir -v -p "$(LIBDIR)"
	$(MAKECMDPREFIX)cp -v "lib/$(LIBNAME)" "$(LIBDIR)/$(LIBNAME).$(VERSION)"
	$(MAKECMDPREFIX)$(STRIP) "$(LIBDIR)/$(LIBNAME).$(VERSION)"
	$(MAKECMDPREFIX)/sbin/ldconfig -n "$(LIBDIR)"

install-headers: all
	$(MAKECMDPREFIX)mkdir -v -p "$(INCLUDEDIR)"
	$(MAKECMDPREFIX)rm -fr "$(INCLUDEDIR)/nmea"
	$(MAKECMDPREFIX)cp -rv include/nmea "$(INCLUDEDIR)"

uninstall:
	$(MAKECMDPREFIX)rm -fv "$(LIBDIR)/$(LIBNAME)" "$(LIBDIR)/$(LIBNAME).$(VERSION)"
	$(MAKECMDPREFIX)/sbin/ldconfig -n "$(LIBDIR)"
	$(MAKECMDPREFIX)rmdir -v -p --ignore-fail-on-non-empty "$(LIBDIR)"

uninstall-headers:
	$(MAKECMDPREFIX)rm -frv "$(INCLUDEDIR)/nmea"
	$(MAKECMDPREFIX)rmdir -v -p --ignore-fail-on-non-empty "$(INCLUDEDIR)"
