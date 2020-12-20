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

NMEALIB_PATH = nmealib
LIBRARY_PATH = wireformat
JAVA_PATH = wireformat-java
include $(LIBRARY_PATH)/Makefile.inc
NMEALIB_NAME := nmea
LIBRARY_NAME := $(PROJECT)


include Makefile.inc

TOPDIR = ../..
include $(TOPDIR)/Makefile.inc


PUD_NMEALIB_STATICALLY_LINKED ?=
CFLAGS += -D_GNU_SOURCE


RESOURCESDIR = ./resources
NMEALIB_LIB = $(NMEALIB_PATH)/lib
LIBRARY_LIB = $(LIBRARY_PATH)/lib
NMEALIB_LIB_STATIC = $(NMEALIB_LIB)/lib$(NMEALIB_NAME).a

ifeq ($(PUD_NMEALIB_STATICALLY_LINKED),)
LIBS += -L $(NMEALIB_LIB) -L $(LIBRARY_LIB) -l$(NMEALIB_NAME) -lm -l$(LIBRARY_NAME) -lgps
else
# libnmea is statically linked
LIBS += -L $(LIBRARY_LIB) -lm -l$(LIBRARY_NAME) -lgps
endif


SUPPORTED = 0
ifeq ($(OS),linux)
SUPPORTED = 1
endif
ifeq ($(OS),android)
SUPPORTED = 1
endif


ifeq ($(SUPPORTED),0)

.PHONY: all default_target install clean

all: default_target

default_target install clean:
	@echo "*** $(PLUGIN_NAME) plugin not supported on $(OS), sorry!"

else

CFLAGS += -DPLUGIN_VER=\"$(PLUGIN_VER)\"

.PHONY: all default_target install uninstall clean doc doc-clean nmealib library java java-instal java-uninstall

all: default_target

ifeq ($(OS),android)
# On Android Google forgot to include regex engine code for Froyo version (but also there was
# no support in older versions for it) so we have here this missing code.
# http://groups.google.com/group/android-ndk/browse_thread/thread/5ea6f0650f0e3fc
OBJS +=     $(REGEX_LIB)
CFLAGS += -D__POSIX_VISIBLE
endif

default_target: nmealib library $(PLUGIN_FULLNAME)

$(PLUGIN_FULLNAME): $(OBJS) version-script.txt
ifeq ($(PUD_NMEALIB_STATICALLY_LINKED),)
ifeq ($(VERBOSE),0)
	@echo "[LD] $@ (nmealib dynamically linked)"
endif
	$(MAKECMDPREFIX)$(CC) $(LDFLAGS) -o $(PLUGIN_FULLNAME) $(OBJS) $(LIBS)
else
ifeq ($(VERBOSE),0)
	@echo "[LD] $@ (nmealib statically linked)"
endif
	$(MAKECMDPREFIX)$(CC) $(LDFLAGS) -o $(PLUGIN_FULLNAME) $(OBJS) $(NMEALIB_LIB_STATIC) $(LIBS)
endif

install: all
	$(MAKECMDPREFIX)$(MAKE) -C "$(NMEALIB_PATH)" DESTDIR="$(DESTDIR)" install
	$(MAKECMDPREFIX)$(MAKE) -C "$(LIBRARY_PATH)" DESTDIR="$(DESTDIR)" install
	$(INSTALL_LIB)
	mkdir -p "$(ETCDIR)"
	cp "$(RESOURCESDIR)/olsrd.pud.position.conf" "$(ETCDIR)"
	$(STRIP) "$(LIBDIR)/$(PLUGIN_FULLNAME)"
ifneq ($(DOCDIR_OLSRD),)
	mkdir -p "$(DOCDIR_OLSRD)"
	cp -t "$(DOCDIR_OLSRD)" "README_PUD" "nmealib/README_NMEALIB" \
	  "wireformat-java/README_PUD_WIREFORMAT_JAVA" "doc/pud.odt"
endif

uninstall:
ifneq ($(DOCDIR_OLSRD),)
	rm -f "$(DOCDIR_OLSRD)/README_PUD" "$(DOCDIR_OLSRD)/README_NMEALIB" \
	  "$(DOCDIR_OLSRD)/README_PUD_WIREFORMAT_JAVA" "$(DOCDIR_OLSRD)/pud.odt"
	rmdir -p --ignore-fail-on-non-empty "$(DOCDIR_OLSRD)"
endif
	$(MAKECMDPREFIX)$(MAKE) -C "$(NMEALIB_PATH)" DESTDIR="$(DESTDIR)" uninstall
	$(MAKECMDPREFIX)$(MAKE) -C "$(LIBRARY_PATH)" DESTDIR="$(DESTDIR)" uninstall
	rm -f "$(LIBDIR)/lib$(PLUGIN_NAME).so" "$(LIBDIR)/$(PLUGIN_NAME)" "$(ETCDIR)/olsrd.pud.position.conf"
	$(UNINSTALL_LIB)
	rmdir -v -p --ignore-fail-on-non-empty "$(LIBDIR)" "$(ETCDIR)"

clean:
ifeq ($(VERBOSE),0)
	@echo "[$@]"
endif
	$(MAKECMDPREFIX)rm -f $(OBJS) $(SRCS:%.c=%.d) "$(PLUGIN_FULLNAME)"
	$(MAKECMDPREFIX)$(MAKE) -C doc clean
	$(MAKECMDPREFIX)$(MAKE) -C "$(NMEALIB_PATH)" clean
	$(MAKECMDPREFIX)$(MAKE) -C "$(LIBRARY_PATH)" clean
	$(MAKECMDPREFIX)$(MAKE) -C "$(JAVA_PATH)" clean

doc:
	$(MAKECMDPREFIX)$(MAKE) -C doc all
	$(MAKECMDPREFIX)$(MAKE) -C "$(NMEALIB_PATH)" doc
	$(MAKECMDPREFIX)$(MAKE) -C "$(LIBRARY_PATH)" doc
	$(MAKECMDPREFIX)$(MAKE) -C "$(JAVA_PATH)" doc

doc-clean:
	$(MAKECMDPREFIX)$(MAKE) -C doc clean
	$(MAKECMDPREFIX)$(MAKE) -C "$(NMEALIB_PATH)" doc clean
	$(MAKECMDPREFIX)$(MAKE) -C "$(LIBRARY_PATH)" doc clean
	$(MAKECMDPREFIX)$(MAKE) -C "$(JAVA_PATH)" doc clean

nmealib:
	$(MAKECMDPREFIX)$(MAKE) -C "$(NMEALIB_PATH)" all

library:
	$(MAKECMDPREFIX)$(MAKE) -C "$(LIBRARY_PATH)" all

java:	library
	$(MAKECMDPREFIX)$(MAKE) -C "$(JAVA_PATH)" all

java-install: java
	$(MAKECMDPREFIX)$(MAKE) -C "$(JAVA_PATH)" DESTDIR="$(DESTDIR)" install

java-uninstall:
	$(MAKECMDPREFIX)$(MAKE) -C "$(JAVA_PATH)" DESTDIR="$(DESTDIR)" uninstall

endif
