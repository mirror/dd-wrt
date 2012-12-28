# The olsr.org Optimized Link-State Routing daemon(olsrd)
# Copyright (c) 2004, Andreas Tonnesen(andreto@olsr.org)
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

# Please also write a new version to:
# gui/win32/Main/Frontend.rc (line 71, around "CAPTION [...]")
# gui/win32/Inst/installer.nsi (line 57, around "MessageBox MB_YESNO [...]")
VERS =		0.6.4

TOPDIR = .
INSTALLOVERWRITE ?=
include Makefile.inc

# pass generated variables to save time
MAKECMD = $(MAKE) OS="$(OS)" WARNINGS="$(WARNINGS)"

LIBS +=		$(OS_LIB_DYNLOAD)
ifeq ($(OS), win32)
LDFLAGS +=	-Wl,--out-implib=libolsrd.a
LDFLAGS +=	-Wl,--export-all-symbols
endif

SWITCHDIR =	src/olsr_switch
CFGDIR =	src/cfgparser
include $(CFGDIR)/local.mk
TAG_SRCS =	$(SRCS) $(HDRS) $(wildcard $(CFGDIR)/*.[ch] $(SWITCHDIR)/*.[ch])

.PHONY: default_target switch
default_target: $(EXENAME)

$(EXENAME):	$(OBJS) src/builddata.o
		@echo "[LD] $@"
		@$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

cfgparser:	$(CFGDEPS) src/builddata.o
		$(MAKE) -C $(CFGDIR)

switch:		
	@$(MAKECMD) -C $(SWITCHDIR)

# generate it always
.PHONY: src/builddata.c
src/builddata.c:
	@$(RM) "$@"
	@echo "#include \"defs.h\"" >> "$@" 
	@echo "const char olsrd_version[] = \"olsr.org -  $(VERS)-git_`git log -1 --pretty=%h`-hash_`./make/hash_source.sh`\";"  >> "$@"
	@date +"const char build_date[] = \"%Y-%m-%d %H:%M:%S\";" >> "$@" 
	@echo "const char build_host[] = \"$(shell hostname)\";" >> "$@" 


.PHONY: help libs clean_libs libs_clean clean distclean uberclean install_libs uninstall_libs libs_install libs_uninstall install_bin uninstall_bin install_olsrd uninstall_olsrd install uninstall build_all install_all uninstall_all clean_all 

clean:
	-rm -f $(OBJS) $(SRCS:%.c=%.d) $(EXENAME) $(EXENAME).exe src/builddata.c $(TMPFILES)
	-rm -f libolsrd.a
	-rm -f olsr_switch.exe
	-rm -f gui/win32/Main/olsrd_cfgparser.lib
	-rm -f olsr-setup.exe
	-rm -fr gui/win32/Main/Release
	-rm -fr gui/win32/Shim/Release

clean_gui:
	$(MAKE) -C gui/linux-gtk clean

distclean: uberclean
uberclean:	clean clean_libs clean_gui
	-rm -f $(TAGFILE)
#	BSD-xargs has no "--no-run-if-empty" aka "-r"
	find . \( -name '*.[od]' -o -name '*~' \) -not -path "*/.hg*" -type f -print0 | xargs -0 rm -f
	@$(MAKECMD) -C $(SWITCHDIR) clean
	@$(MAKECMD) -C $(CFGDIR) clean

install: install_olsrd

uninstall: uninstall_olsrd

install_bin:
		mkdir -p $(SBINDIR)
		install -m 755 $(EXENAME) $(SBINDIR)
		$(STRIP) $(SBINDIR)/$(EXENAME)

uninstall_bin:
		rm -f $(SBINDIR)/$(EXENAME)
		rmdir -p --ignore-fail-on-non-empty $(SBINDIR)

install_olsrd:	install_bin
		@echo ========= C O N F I G U R A T I O N - F I L E ============
		@echo $(EXENAME) uses the configfile $(CFGFILE)
		@echo a default configfile. A sample RFC-compliance aimed
		@echo configfile can be found in olsrd.conf.default.rfc.
		@echo However none of the larger OLSRD using networks use that
		@echo so install a configfile with activated link quality exstensions
		@echo per default.
		@echo can be found at files/olsrd.conf.default.lq
		@echo ==========================================================
		mkdir -p $(ETCDIR)
		@if [ -e $(CFGFILE) ]; then \
			cp -f files/olsrd.conf.default.lq $(CFGFILE).new; \
			echo "Configuration file was saved as $(CFGFILE).new"; \
		else \
			cp -f files/olsrd.conf.default.lq $(CFGFILE); \
		fi
		@echo -------------------------------------------
		@echo Edit $(CFGFILE) before running olsrd!!
		@echo -------------------------------------------
		@echo Installing manpages $(EXENAME)\(8\) and $(CFGNAME)\(5\)
ifneq ($(MANDIR),)
		mkdir -p $(MANDIR)/man8/
		cp files/olsrd.8.gz $(MANDIR)/man8/$(EXENAME).8.gz
		mkdir -p $(MANDIR)/man5/
		cp files/olsrd.conf.5.gz $(MANDIR)/man5/$(CFGNAME).5.gz
endif

uninstall_olsrd:	uninstall_bin
ifneq ($(MANDIR),)
		rm -f $(MANDIR)/man5/$(CFGNAME).5.gz
		rmdir -p --ignore-fail-on-non-empty $(MANDIR)/man5/
		rm -f $(MANDIR)/man8/$(EXENAME).8.gz
		rmdir -p --ignore-fail-on-non-empty $(MANDIR)/man8/
endif
		rm -f $(CFGFILE) $(CFGFILE).new
		rmdir -p --ignore-fail-on-non-empty $(ETCDIR)

tags:
		$(TAGCMD) -o $(TAGFILE) $(TAG_SRCS)

rpm:
		@$(RM) olsrd-current.tar.bz2
		@echo "Creating olsrd-current.tar.bz2 ..."
		@./list-excludes.sh | tar  --exclude-from=- --exclude="olsrd-current.tar.bz2" -C .. -cjf olsrd-current.tar.bz2 olsrd-current
		@echo "Building RPMs..."
		@rpmbuild -ta olsrd-current.tar.bz2
#
# PLUGINS
#

# This is quite ugly but at least it works
ifeq ($(OS),linux)
SUBDIRS = arprefresh bmf dot_draw dyn_gw dyn_gw_plain httpinfo jsoninfo mdns mini nameservice p2pd pgraph pud quagga secure sgwdynspeed tas txtinfo watchdog
else
ifeq ($(OS),win32)
SUBDIRS := dot_draw httpinfo jsoninfo mini pgraph secure txtinfo
else
ifeq ($(OS),android)
SUBDIRS := arprefresh bmf dot_draw dyn_gw_plain httpinfo jsoninfo mini nameservice pgraph pud secure sgwdynspeed tas txtinfo watchdog
else
SUBDIRS := dot_draw dyn_gw dyn_gw_plain httpinfo jsoninfo mini nameservice pgraph secure txtinfo watchdog
endif
endif
endif

libs:
		@set -e;for dir in $(SUBDIRS);do $(MAKECMD) -C lib/$$dir LIBDIR=$(LIBDIR);done

libs_clean clean_libs:
		-for dir in $(SUBDIRS);do $(MAKECMD) -C lib/$$dir LIBDIR=$(LIBDIR) clean;rm -f lib/$$dir/*.so lib/$$dir/*.dll;done
		-rm -f $(REGEX_OBJS)

libs_install install_libs:
		@set -e;for dir in $(SUBDIRS);do $(MAKECMD) -C lib/$$dir LIBDIR=$(LIBDIR) install;done

libs_uninstall uninstall_libs:
		@set -e;for dir in $(SUBDIRS);do $(MAKECMD) -C lib/$$dir LIBDIR=$(LIBDIR) uninstall;done
		rmdir -p --ignore-fail-on-non-empty $(LIBDIR)

#
# PLUGINS
#

arprefresh:
		@$(MAKECMD) -C lib/arprefresh

arprefresh_clean:
		@$(MAKECMD) -C lib/arprefresh DESTDIR=$(DESTDIR) clean

arprefresh_install:
		@$(MAKECMD) -C lib/arprefresh DESTDIR=$(DESTDIR) install

arprefresh_uninstall:
		@$(MAKECMD) -C lib/arprefresh DESTDIR=$(DESTDIR) uninstall

bmf:
		@$(MAKECMD) -C lib/bmf

bmf_clean:
		@$(MAKECMD) -C lib/bmf DESTDIR=$(DESTDIR) clean

bmf_install:
		@$(MAKECMD) -C lib/bmf DESTDIR=$(DESTDIR) install

bmf_uninstall:
		@$(MAKECMD) -C lib/bmf DESTDIR=$(DESTDIR) uninstall

dot_draw:
		@$(MAKECMD) -C lib/dot_draw

dot_draw_clean:
		@$(MAKECMD) -C lib/dot_draw DESTDIR=$(DESTDIR) clean

dot_draw_install:
		@$(MAKECMD) -C lib/dot_draw DESTDIR=$(DESTDIR) install

dot_draw_uninstall:
		@$(MAKECMD) -C lib/dot_draw DESTDIR=$(DESTDIR) uninstall

dyn_gw:
		@$(MAKECMD) -C lib/dyn_gw

dyn_gw_clean:
		@$(MAKECMD) -C lib/dyn_gw DESTDIR=$(DESTDIR) clean

dyn_gw_install:
		@$(MAKECMD) -C lib/dyn_gw DESTDIR=$(DESTDIR) install

dyn_gw_uninstall:
		@$(MAKECMD) -C lib/dyn_gw DESTDIR=$(DESTDIR) uninstall

dyn_gw_plain:
		@$(MAKECMD) -C lib/dyn_gw_plain

dyn_gw_plain_clean:
		@$(MAKECMD) -C lib/dyn_gw_plain DESTDIR=$(DESTDIR) clean

dyn_gw_plain_install:
		@$(MAKECMD) -C lib/dyn_gw_plain DESTDIR=$(DESTDIR) install

dyn_gw_plain_uninstall:
		@$(MAKECMD) -C lib/dyn_gw_plain DESTDIR=$(DESTDIR) uninstall

httpinfo:
		@$(MAKECMD) -C lib/httpinfo

httpinfo_clean:
		@$(MAKECMD) -C lib/httpinfo DESTDIR=$(DESTDIR) clean

httpinfo_install:
		@$(MAKECMD) -C lib/httpinfo DESTDIR=$(DESTDIR) install

httpinfo_uninstall:
		@$(MAKECMD) -C lib/httpinfo DESTDIR=$(DESTDIR) uninstall

jsoninfo:
		@$(MAKECMD) -C lib/jsoninfo

jsoninfo_clean:
		@$(MAKECMD) -C lib/jsoninfo DESTDIR=$(DESTDIR) clean

jsoninfo_install:
		@$(MAKECMD) -C lib/jsoninfo DESTDIR=$(DESTDIR) install

jsoninfo_uninstall:
		@$(MAKECMD) -C lib/jsoninfo DESTDIR=$(DESTDIR) uninstall

mdns:
		@$(MAKECMD) -C lib/mdns

mdns_clean:
		@$(MAKECMD) -C lib/mdns DESTDIR=$(DESTDIR) clean

mdns_install:
		@$(MAKECMD) -C lib/mdns DESTDIR=$(DESTDIR) install

mdns_uninstall:
		@$(MAKECMD) -C lib/mdns DESTDIR=$(DESTDIR) uninstall

#
# no targets for mini: it's an example plugin
#

# nameserver uses regex, which was only recently added to Android.  On
# Android, $(REGEX_OBJS) will have all of the files needed, on all
# other platforms, it'll be empty and therefore ignored.
nameservice: $(REGEX_OBJS)
		@$(MAKECMD) -C lib/nameservice clean
		@$(MAKECMD) -C lib/nameservice

nameservice_clean:
		@$(MAKECMD) -C lib/nameservice DESTDIR=$(DESTDIR) clean

nameservice_install:
		@$(MAKECMD) -C lib/nameservice DESTDIR=$(DESTDIR) install

nameservice_uninstall:
		@$(MAKECMD) -C lib/nameservice DESTDIR=$(DESTDIR) uninstall

p2pd:
		@$(MAKECMD) -C lib/p2pd

p2pd_clean:
		@$(MAKECMD) -C lib/p2pd DESTDIR=$(DESTDIR) clean

p2pd_install:
		@$(MAKECMD) -C lib/p2pd DESTDIR=$(DESTDIR) install

p2pd_uninstall:
		@$(MAKECMD) -C lib/p2pd DESTDIR=$(DESTDIR) uninstall

pgraph:
		@$(MAKECMD) -C lib/pgraph

pgraph_clean:
		@$(MAKECMD) -C lib/pgraph DESTDIR=$(DESTDIR) clean

pgraph_install:
		@$(MAKECMD) -C lib/pgraph DESTDIR=$(DESTDIR) install

pgraph_uninstall:
		@$(MAKECMD) -C lib/pgraph DESTDIR=$(DESTDIR) uninstall

pud:
		@$(MAKECMD) -C lib/pud

pud_clean:
		@$(MAKECMD) -C lib/pud DESTDIR=$(DESTDIR) clean

pud_install:
		@$(MAKECMD) -C lib/pud DESTDIR=$(DESTDIR) install

pud_uninstall:
		@$(MAKECMD) -C lib/pud DESTDIR=$(DESTDIR) uninstall

quagga:
		@$(MAKECMD) -C lib/quagga

quagga_clean:
		@$(MAKECMD) -C lib/quagga DESTDIR=$(DESTDIR) clean

quagga_install:
		@$(MAKECMD) -C lib/quagga DESTDIR=$(DESTDIR) install

quagga_uninstall:
		@$(MAKECMD) -C lib/quagga DESTDIR=$(DESTDIR) uninstall

secure:
		@$(MAKECMD) -C lib/secure

secure_clean:
		@$(MAKECMD) -C lib/secure DESTDIR=$(DESTDIR) clean

secure_install:
		@$(MAKECMD) -C lib/secure DESTDIR=$(DESTDIR) install

secure_uninstall:
		@$(MAKECMD) -C lib/secure DESTDIR=$(DESTDIR) uninstall

sgwdynspeed:
		@$(MAKECMD) -C lib/sgwdynspeed

sgwdynspeed_clean:
		@$(MAKECMD) -C lib/sgwdynspeed DESTDIR=$(DESTDIR) clean

sgwdynspeed_install:
		@$(MAKECMD) -C lib/sgwdynspeed DESTDIR=$(DESTDIR) install

sgwdynspeed_uninstall:
		@$(MAKECMD) -C lib/sgwdynspeed DESTDIR=$(DESTDIR) uninstall

tas:
		@$(MAKECMD) -C lib/tas

tas_clean:
		@$(MAKECMD) -C lib/tas DESTDIR=$(DESTDIR) clean

tas_install:
		@$(MAKECMD) -C lib/tas DESTDIR=$(DESTDIR) install

tas_uninstall:
		@$(MAKECMD) -C lib/tas DESTDIR=$(DESTDIR) uninstall

txtinfo:
		@$(MAKECMD) -C lib/txtinfo

txtinfo_clean:
		@$(MAKECMD) -C lib/txtinfo DESTDIR=$(DESTDIR) clean

txtinfo_install:
		@$(MAKECMD) -C lib/txtinfo DESTDIR=$(DESTDIR) install

txtinfo_uninstall:
		@$(MAKECMD) -C lib/txtinfo DESTDIR=$(DESTDIR) uninstall

watchdog:
		@$(MAKECMD) -C lib/watchdog

watchdog_clean:
		@$(MAKECMD) -C lib/watchdog DESTDIR=$(DESTDIR) clean

watchdog_install:
		@$(MAKECMD) -C lib/watchdog DESTDIR=$(DESTDIR) install

watchdog_uninstall:
		@$(MAKECMD) -C lib/watchdog DESTDIR=$(DESTDIR) uninstall


build_all:	all switch libs
install_all:	install install_libs
uninstall_all:	uninstall uninstall_libs
clean_all:	uberclean clean_libs
