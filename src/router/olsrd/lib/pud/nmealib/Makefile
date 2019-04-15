include ../Makefile.inc
OLSRD_PLUGIN = false

TOPDIR = ../../..
include $(TOPDIR)/Makefile.inc


include Makefile.inc

#
# Settings
#

DESTDIR ?=
USRDIR ?= $(DESTDIR)/usr
INCLUDEDIR ?= $(DESTDIR)/usr/include
LIBDIR ?= $(USRDIR)/lib

H_FILES = $(sort $(wildcard include/nmealib/*.h))
C_FILES = $(sort $(wildcard src/*.c))

MODULES = $(C_FILES:src/%.c=%)

OBJ = $(MODULES:%=build/%.o)

LIBRARIES = -lm
INCLUDES = -I ./include


.PRECIOUS: $(OBJ)


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

build/%.o: src/%.c $(H_FILES) Makefile Makefile.inc
ifeq ($(VERBOSE),0)
	@echo "[CC] $<"
endif
	$(MAKECMDPREFIX)$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

full : all


#
# Phony Targets
#

.PHONY: all-before clean doc doc-pdf doc-all doc-clean install install-headers uninstall uninstall-headers

all-before:
	$(MAKECMDPREFIX)mkdir -p build lib

clean:
	$(MAKECMDPREFIX)$(MAKE) -C doc clean
	$(MAKECMDPREFIX)rm -fr build lib

doc:
	$(MAKECMDPREFIX)$(MAKE) -C doc doc

doc-pdf:
	$(MAKECMDPREFIX)$(MAKE) -C doc doc-pdf

doc-all:
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
	$(MAKECMDPREFIX)rm -fr "$(INCLUDEDIR)/nmealib"
	$(MAKECMDPREFIX)cp -rv include/nmealib "$(INCLUDEDIR)"

uninstall:
	$(MAKECMDPREFIX)rm -fv "$(LIBDIR)/$(LIBNAME)" "$(LIBDIR)/$(LIBNAME).$(VERSION)"
	$(MAKECMDPREFIX)/sbin/ldconfig -n "$(LIBDIR)"
	$(MAKECMDPREFIX)rmdir -v -p --ignore-fail-on-non-empty "$(LIBDIR)"

uninstall-headers:
	$(MAKECMDPREFIX)rm -frv "$(INCLUDEDIR)/nmealib"
	$(MAKECMDPREFIX)rmdir -v -p --ignore-fail-on-non-empty "$(INCLUDEDIR)"

