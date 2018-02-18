# The olsr.org Optimized Link-State Routing daemon (olsrd)
#
# (c) by the OLSR project
#
# See our Git repository to find out who worked on this file
# and thus is a copyright holder on it.
#
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# * Redistributions of source code must retain the above copyright
#   notice, this list of conditions and the following disclaimer.
# * Redistributions in binary form must reproduce the above copyright
#   notice, this list of conditions and the following disclaimer in
#   the documentation and/or other materials provided with the
#   distribution.
# * Neither the name of olsr.org, olsrd nor the names of its
#   contributors may be used to endorse or promote products derived
#   from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
# COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#
# Visit http://www.olsr.org for more information.
#
# If you find this software useful feel free to make a donation
# to the project. For more information see the website or contact
# the copyright holders.
#

include Makefile.inc

include ../Makefile.inc
OLSRD_PLUGIN = false

TOPDIR = ../../..
include $(TOPDIR)/Makefile.inc

LDFLAGS = -shared -Wl,--warn-common -fPIC

# 32/64 cross compilation
ifdef M32
CFLAGS +=	-m32
LDFLAGS +=	-m32
endif
ifdef M64
CFLAGS +=	-m64
LDFLAGS +=	-m64
endif

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

MODULES = nodeIdConversion wireFormat

SRC_DIR_C = src
OBJECTS_C = $(MODULES:%=$(BUILD_DIR)/%.o)

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
	$(MAKECMDPREFIX)$(CC) $(LDFLAGS) -Wl,-soname=$(SONAME) $(LIBRARIES) -o "$@" $(OBJECTS_C)

$(BUILD_DIR)/%.o: $(SRC_DIR_C)/%.c $(INC_DIR_C)/compiler.h
ifeq ($(VERBOSE),0)
	@echo "[CC] $<"
endif
	$(MAKECMDPREFIX)$(CC) $(CFLAGS) $(CPPFLAGS) -c -o "$@" "$<"

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
