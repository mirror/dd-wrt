include ../Makefile.inc
OLSRD_PLUGIN = false

TOPDIR = ../../..
include $(TOPDIR)/Makefile.inc


include Makefile.inc

#
# Settings
#

LIBNAME = libnmea.so

DESTDIR ?=

MACHINE=$(shell uname -m)
LIBDIR_INSTALL = usr/lib
INCLUDEDIR = usr/include

MODULES = context generate generator gmath info parse parser sentence time tok util
OBJ = $(MODULES:%=build/%.o)

LIBRARIES = -lm
INCLUDES = -I ./include


#
# Targets
#

all: default_target

default_target: all-before lib/$(LIBNAME)

remake: clean all

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
	rm -fr build lib

doc:
	$(MAKE) -C doc all

install: all
	@mkdir -v -p "$(DESTDIR)/$(LIBDIR_INSTALL)"
	cp "lib/$(LIBNAME)" "$(DESTDIR)/$(LIBDIR_INSTALL)/$(LIBNAME).$(VERSION)"
	$(STRIP) "$(DESTDIR)/$(LIBDIR_INSTALL)/$(LIBNAME).$(VERSION)"
	ldconfig -n "$(DESTDIR)/$(LIBDIR_INSTALL)"

install-headers: all
	@mkdir -v -p "$(DESTDIR)/$(INCLUDEDIR)"
	@rm -fr "$(DESTDIR)/$(INCLUDEDIR)/nmea"
	cp -r include/nmea "$(DESTDIR)/$(INCLUDEDIR)"

uninstall:
	rm -f "$(DESTDIR)/$(LIBDIR_INSTALL)/$(LIBNAME)" "$(DESTDIR)/$(LIBDIR_INSTALL)/$(LIBNAME).$(VERSION)"
	ldconfig -n "$(DESTDIR)/$(LIBDIR_INSTALL)"
	@rmdir -v -p --ignore-fail-on-non-empty "$(DESTDIR)/$(LIBDIR_INSTALL)"

uninstall-headers:
	rm -fr "$(DESTDIR)/$(INCLUDEDIR)/nmea"
	@rmdir -v -p --ignore-fail-on-non-empty "$(DESTDIR)/$(INCLUDEDIR)"
