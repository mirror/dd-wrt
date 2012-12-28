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
	@echo "[AR] $@"
	@ar rcs "$@" $(OBJ)

lib/$(LIBNAME): $(OBJ)
	@echo "[LD] $@"
	@$(CC) -shared -Wl,-soname=$(LIBNAME) -o "$@" $(LIBRARIES) $(OBJ)

build/%.o: src/%.c Makefile Makefile.inc
	@echo "[CC] $<"
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

samples: all
	@$(MAKE) -C samples all


#
# Phony Targets
#

.PHONY: all default_target all-before clean doc install install-headers uninstall uninstall-headers

all-before:
	@mkdir -p build lib

clean:
	@$(MAKE) -C doc clean
	@$(MAKE) -C samples clean
	@rm -frv build lib

doc:
	@$(MAKE) -C doc all

doc-clean:
	@$(MAKE) -C doc clean

install: all
	@mkdir -v -p "$(LIBDIR)"
	@cp -v "lib/$(LIBNAME)" "$(LIBDIR)/$(LIBNAME).$(VERSION)"
	@$(STRIP) "$(LIBDIR)/$(LIBNAME).$(VERSION)"
	@/sbin/ldconfig -n "$(LIBDIR)"

install-headers: all
	@mkdir -v -p "$(INCLUDEDIR)"
	@rm -fr "$(INCLUDEDIR)/nmea"
	@cp -rv include/nmea "$(INCLUDEDIR)"

uninstall:
	@rm -fv "$(LIBDIR)/$(LIBNAME)" "$(LIBDIR)/$(LIBNAME).$(VERSION)"
	@/sbin/ldconfig -n "$(LIBDIR)"
	@rmdir -v -p --ignore-fail-on-non-empty "$(LIBDIR)"

uninstall-headers:
	@rm -frv "$(INCLUDEDIR)/nmea"
	@rmdir -v -p --ignore-fail-on-non-empty "$(INCLUDEDIR)"
