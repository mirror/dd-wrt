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

MACHINE=$(shell uname -m)
LIBDIR_INSTALL = usr/lib

BUILD_DIR = build

INC_DIR_BASE_C = include
INC_DIR_C = $(INC_DIR_BASE_C)/$(PROJECT)
INC_DIR_OLSRD = ../../../src

LIBDIR_BUILD = lib

MODULES = nodeIdConversion wireFormat

SRC_DIR_C = src
OBJECTS_C = $(MODULES:%=$(BUILD_DIR)/%.o)

INCLUDES = -I "$(INC_DIR_BASE_C)" -I "$(INC_DIR_OLSRD)"
LIBRARIES = -lm

DESTDIR ?=
INC_DIR_INSTALL = usr/$(INC_DIR_C)


ifneq ($(OS),linux)

.PHONY: all default_target install clean

all: default_target

default_target install clean:
	@echo "*** only supported on Linux"

else

.PHONY: all default_target prepare clean doc doc-clean install install-headers uninstall uninstall-headers

#
# Targets
#

all: default_target

default_target: prepare $(LIBDIR_BUILD)/$(SONAME)

$(LIBDIR_BUILD)/$(SONAME): $(OBJECTS_C)
	@echo "[LD] $@"
	@$(CC) -shared -Wl,-soname=$(SONAME) $(LIBRARIES) -o "$@" $(OBJECTS_C)

$(BUILD_DIR)/%.o: $(SRC_DIR_C)/%.c $(INC_DIR_C)/compiler.h
	@echo "[CC] $<"
	@$(CC) $(CFLAGS) $(CPPFLAGS) $(INCLUDES) -c -o "$@" "$<"

.gitignore:
	@echo "[$@]"
	@echo "/$(BUILD_DIR)/" > $@
	@echo "/$(LIBDIR_BUILD)/" >> $@
	@$(MAKE) -C doc $@


#
# Phony Targets
#

prepare:
	@mkdir -p "$(BUILD_DIR)" "$(LIBDIR_BUILD)"

clean: doc-clean
	@echo "[$@]"
	@rm -fr "$(BUILD_DIR)" "$(LIBDIR_BUILD)" "$(SRC_DIR_C)/"*.d

doc:
	@$(MAKE) -C doc all

doc-clean:
	@$(MAKE) -C doc clean

install: all
	mkdir -v -p "$(DESTDIR)/$(LIBDIR_INSTALL)"
	cp "$(LIBDIR_BUILD)/$(SONAME)" "$(DESTDIR)/$(LIBDIR_INSTALL)/$(SONAME).$(PLUGIN_VER)"
	$(STRIP) "$(DESTDIR)/$(LIBDIR_INSTALL)/$(SONAME).$(PLUGIN_VER)"
	ldconfig -n "$(DESTDIR)/$(LIBDIR_INSTALL)"

install-headers:
	rm -fr "$(DESTDIR)/$(INC_DIR_INSTALL)"
	mkdir -v -p "$(DESTDIR)/$(INC_DIR_INSTALL)"
	cp -t "$(DESTDIR)/$(INC_DIR_INSTALL)" $(INC_DIR_C)/*.h

uninstall:
	rm -f "$(DESTDIR)/$(LIBDIR_INSTALL)/$(SONAME)" \
	      "$(DESTDIR)/$(LIBDIR_INSTALL)/$(SONAME).$(PLUGIN_VER)"
	ldconfig -n "$(DESTDIR)/$(LIBDIR_INSTALL)"
	rmdir -v -p --ignore-fail-on-non-empty \
	  "$(DESTDIR)/$(LIBDIR_INSTALL)"

uninstall-headers:
	rm -fr "$(DESTDIR)/$(INC_DIR_INSTALL)/"*.h
	rmdir -v -p --ignore-fail-on-non-empty \
	  "$(DESTDIR)/$(INC_DIR_INSTALL)"

endif
