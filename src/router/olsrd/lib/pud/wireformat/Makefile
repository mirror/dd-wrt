include Makefile.inc

include ../Makefile.inc
OLSRD_PLUGIN = false

TOPDIR = ../../..
include $(TOPDIR)/Makefile.inc

######################
#
# Settings
#
######################

DESTDIR ?=
USRDIR ?= $(DESTDIR)/usr
INCLUDEDIR ?= $(DESTDIR)/usr/include
LIBDIR ?= $(USRDIR)/lib

BUILD_DIR = build
LIBDIR_BUILD = lib


INC_DIR_BASE_C = ./include
INC_DIR_C = $(INC_DIR_BASE_C)/$(PROJECT)
INC_DIR_OLSRD = ../../../src

MODULES = nodeIdConversion wireFormat

SRC_DIR_C = src
OBJECTS_C = $(MODULES:%=$(BUILD_DIR)/%.o)

INCLUDES = -I "$(INC_DIR_BASE_C)" -I "$(INC_DIR_OLSRD)"
LIBRARIES = -lm


.PHONY: all default_target prepare clean doc doc-clean install install-headers uninstall uninstall-headers

#
# Targets
#

all: default_target

default_target: prepare $(LIBDIR_BUILD)/$(SONAME)

$(LIBDIR_BUILD)/$(SONAME): $(OBJECTS_C)
ifeq ($(VERBOSE),0)
	@echo "[LD] $@"
endif
	$(MAKECMDPREFIX)$(CC) -shared -Wl,-soname=$(SONAME) $(LIBRARIES) -o "$@" $(OBJECTS_C)

$(BUILD_DIR)/%.o: $(SRC_DIR_C)/%.c $(INC_DIR_C)/compiler.h
ifeq ($(VERBOSE),0)
	@echo "[CC] $<"
endif
	$(MAKECMDPREFIX)$(CC) $(CFLAGS) $(CPPFLAGS) $(INCLUDES) -c -o "$@" "$<"

.gitignore:
ifeq ($(VERBOSE),0)
	@echo "[$@]"
endif
	$(MAKECMDPREFIX)echo "/$(BUILD_DIR)/" > $@
	$(MAKECMDPREFIX)echo "/$(LIBDIR_BUILD)/" >> $@
	$(MAKECMDPREFIX)$(MAKE) -C doc $@


#
# Phony Targets
#

prepare:
	$(MAKECMDPREFIX)mkdir -p "$(BUILD_DIR)" "$(LIBDIR_BUILD)"

clean: doc-clean
ifeq ($(VERBOSE),0)
	@echo "[$@]"
endif
	$(MAKECMDPREFIX)rm -fr "$(BUILD_DIR)" "$(LIBDIR_BUILD)" "$(SRC_DIR_C)/"*.d

doc:
	$(MAKECMDPREFIX)$(MAKE) -C doc all

doc-clean:
	$(MAKECMDPREFIX)$(MAKE) -C doc clean

install: all
	mkdir -v -p "$(LIBDIR)"
	cp "$(LIBDIR_BUILD)/$(SONAME)" "$(LIBDIR)/$(SONAME).$(PLUGIN_VER)"
	$(STRIP) "$(LIBDIR)/$(SONAME).$(PLUGIN_VER)"
	/sbin/ldconfig -n "$(LIBDIR)"

install-headers:
	rm -fr "$(INCLUDEDIR)/$(PROJECT)"
	mkdir -v -p "$(INCLUDEDIR)/$(PROJECT)"
	cp -t "$(INCLUDEDIR)/$(PROJECT)" $(INC_DIR_C)/*.h

uninstall:
	rm -f "$(LIBDIR)/$(SONAME)" \
	      "$(LIBDIR)/$(SONAME).$(PLUGIN_VER)"
	/sbin/ldconfig -n "$(LIBDIR)"
	rmdir -v -p --ignore-fail-on-non-empty \
	  "$(LIBDIR)"

uninstall-headers:
	rm -fr "$(INCLUDEDIR)/$(PROJECT)/"*.h
	rmdir -v -p --ignore-fail-on-non-empty \
	  "$(INCLUDEDIR)/$(PROJECT)"
