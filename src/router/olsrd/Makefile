# The olsr.org Optimized Link-State Routing daemon(olsrd)
# Copyright (c) 2004, Andreas Tønnesen(andreto@olsr.org)
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
# $Id: Makefile,v 1.84 2007/05/01 21:36:50 bernd67 Exp $

VERS =		0.5.1pre

TOPDIR = .
include Makefile.inc

CFLAGS +=	-DVERSION=\"$(VERS)\"

MAKECMD = $(MAKE) OS="$(OS)" WARNINGS="$(WARNINGS)"

LIBS +=		$(OS_LIB_DYNLOAD)

ifeq ($(OS), win32)
LDFLAGS +=	-Wl,--out-implib=libolsrd.a -Wl,--export-all-symbols
endif

SWITCHDIR =     src/olsr_switch
CFGDIR =	src/cfgparser
CFGOBJS = 	$(CFGDIR)/oscan.o $(CFGDIR)/oparse.o $(CFGDIR)/olsrd_conf.o
CFGDEPS = 	$(wildcard $(CFGDIR)/*.c) $(wildcard $(CFGDIR)/*.h) $(CFGDIR)/oparse.y $(CFGDIR)/oscan.lex
TAG_SRCS = $(SRCS) $(HDRS) $(wildcard src/cfgparser/*.c) $(wildcard src/cfgparser/*.h) $(wildcard src/olsr_switch/*.c) $(wildcard src/olsr_switch/*.h)

default_target: cfgparser olsrd

olsrd:		$(OBJS) $(CFGOBJS)
		$(CC) $(LDFLAGS) -o $@ $(OBJS) $(CFGOBJS) $(LIBS)

cfgparser:	$(CFGDEPS)
		$(MAKECMD) -C $(CFGDIR)

switch:		
		$(MAKECMD) -C $(SWITCHDIR)

$(CFGOBJS):
		$(MAKECMD) -C $(CFGDIR)

.PHONY: help libs clean_libs libs_clean clean uberclean install_libs libs_install install_bin install_olsrd install build_all install_all clean_all

clean:
		-rm -f $(OBJS) $(SRCS:%.c=%.d) olsrd olsrd.exe
		$(MAKECMD) -C $(CFGDIR) clean
		$(MAKECMD) -C $(SWITCHDIR) clean

uberclean:	clean clean_libs
		-rm -f $(TAGFILE)
		find . \( -name '*.[od]' -o -name '*~' \) -print | xargs -r rm -f
		$(MAKECMD) -C $(CFGDIR) uberclean
		$(MAKECMD) -C $(SWITCHDIR) clean

install: install_olsrd

install_bin:
		mkdir -p $(SBINDIR)
		install -m 755 $(EXENAME) $(SBINDIR)
		$(STRIP) $(SBINDIR)/$(EXENAME)

install_olsrd:	install_bin
		@echo ========= C O N F I G U R A T I O N - F I L E ============
		@echo olsrd uses the configfile $(CFGFILE)
		@echo a default configfile. A sample RFC-compliance aimed
		@echo configfile can be installed. Note that a LQ-based configfile
		@echo can be found at files/olsrd.conf.default.lq
		@echo ==========================================================
		mkdir -p $(ETCDIR)
		-cp -i files/olsrd.conf.default.rfc $(CFGFILE)
		@echo -------------------------------------------
		@echo Edit $(CFGFILE) before running olsrd!!
		@echo -------------------------------------------
		@echo Installing manpages olsrd\(8\) and olsrd.conf\(5\)
		mkdir -p $(MANDIR)/man8/
		cp files/olsrd.8.gz $(MANDIR)/man8/olsrd.8.gz
		mkdir -p $(MANDIR)/man5/
		cp files/olsrd.conf.5.gz $(MANDIR)/man5/olsrd.conf.5.gz

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

libs: 
		$(MAKECMD) -C lib LIBDIR=$(LIBDIR)

libs_clean clean_libs:
		$(MAKECMD) -C lib LIBDIR=$(LIBDIR) clean

libs_install install_libs:
		$(MAKECMD) -C lib LIBDIR=$(LIBDIR) install

httpinfo:
		$(MAKECMD) -C lib/httpinfo clean
		$(MAKECMD) -C lib/httpinfo 
		$(MAKECMD) -C lib/httpinfo DESTDIR=$(DESTDIR) install 

tas:
		$(MAKECMD) -C lib/tas clean
		$(MAKECMD) -C lib/tas DESTDIR=$(DESTDIR) install

dot_draw:
		$(MAKECMD) -C lib/dot_draw clean
		$(MAKECMD) -C lib/dot_draw DESTDIR=$(DESTDIR) install

nameservice:
		$(MAKECMD) -C lib/nameservice clean
		$(MAKECMD) -C lib/nameservice DESTDIR=$(DESTDIR) install

dyn_gw:
		$(MAKECMD) -C lib/dyn_gw clean
		$(MAKECMD) -C lib/dyn_gw
		$(MAKECMD) -C lib/dyn_gw DESTDIR=$(DESTDIR) install

dyn_gw_plain:
		$(MAKECMD) -C lib/dyn_gw_plain clean
		$(MAKECMD) -C lib/dyn_gw_plain
		$(MAKECMD) -C lib/dyn_gw_plain DESTDIR=$(DESTDIR) install

secure:
		$(MAKECMD) -C lib/secure clean
		$(MAKECMD) -C lib/secure
		$(MAKECMD) -C lib/secure DESTDIR=$(DESTDIR) install

pgraph:
		$(MAKECMD) -C lib/pgraph clean
		$(MAKECMD) -C lib/pgraph 
		$(MAKECMD) -C lib/pgraph DESTDIR=$(DESTDIR) install 

bmf:
		$(MAKECMD) -C lib/bmf clean
		$(MAKECMD) -C lib/bmf 
		$(MAKECMD) -C lib/bmf DESTDIR=$(DESTDIR) install 

quagga:
		$(MAKECMD) -C lib/quagga clean
		$(MAKECMD) -C lib/quagga 
		$(MAKECMD) -C lib/quagga DESTDIR=$(DESTDIR) install 


build_all:	cfgparser olsrd libs
install_all:	install install_libs
clean_all:	uberclean clean_libs
