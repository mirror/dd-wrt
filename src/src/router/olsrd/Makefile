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
# $Id: Makefile,v 1.67 2005/12/29 21:26:31 tlopatic Exp $

VERS =		0.4.10

TOPDIR = .
include Makefile.inc

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
		$(MAKE) -C $(CFGDIR)

switch:		
		$(MAKE) -C $(SWITCHDIR)

$(CFGOBJS):
		$(MAKE) -C $(CFGDIR)

.PHONY: help libs clean_libs libs_clean clean uberclean install_libs libs_install install_bin install_olsrd install build_all install_all

clean:
		-rm -f $(OBJS) $(SRCS:%.c=%.d) olsrd olsrd.exe $(TAGFILE)
		$(MAKE) -C $(CFGDIR) clean
		$(MAKE) -C $(SWITCHDIR) clean

uberclean:	clean clean_libs
		-rm -f src/*.[od~] 
		-rm -f src/linux/*.[od~] src/unix/*.[od~] src/win32/*.[od~] src/bsd/*.[od~]
		$(MAKE) -C $(CFGDIR) uberclean
		$(MAKE) -C $(SWITCHDIR) clean

install: install_olsrd

install_bin:
		$(STRIP) $(EXENAME)
		mkdir -p $(SBINDIR)
		install -m 755 $(EXENAME) $(SBINDIR)

install_olsrd:	install_bin
		@echo ========= C O N F I G U R A T I O N - F I L E ============
		@echo olsrd uses the configfile $(INSTALL_PREFIX)/etc/olsr.conf
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

#
# PLUGINS
#

libs: 
		$(MAKE) -C lib LIBDIR=$(LIBDIR)

libs_clean clean_libs:
		$(MAKE) -C lib LIBDIR=$(LIBDIR) clean

libs_install install_libs:
		$(MAKE) -C lib LIBDIR=$(LIBDIR) install

httpinfo:
		$(MAKE) -C lib/httpinfo clean
		$(MAKE) -C lib/httpinfo 
		$(MAKE) -C lib/httpinfo install 

tas:
		$(MAKE) -C lib/tas clean
		$(MAKE) -C lib/tas install

dot_draw:
		$(MAKE) -C lib/dot_draw clean
		$(MAKE) -C lib/dot_draw install

nameservice:
		$(MAKE) -C lib/nameservice clean
		$(MAKE) -C lib/nameservice install

dyn_gw:
		$(MAKE) -C lib/dyn_gw clean
		$(MAKE) -C lib/dyn_gw
		$(MAKE) -C lib/dyn_gw install

dyn_gw_plain:
		$(MAKE) -C lib/dyn_gw_plain clean
		$(MAKE) -C lib/dyn_gw_plain
		$(MAKE) -C lib/dyn_gw_plain install

powerinfo:
		$(MAKE) -C lib/powerinfo clean
		$(MAKE) -C lib/powerinfo 
		$(MAKE) -C lib/powerinfo install

secure:
		$(MAKE) -C lib/secure clean
		$(MAKE) -C lib/secure
		$(MAKE) -C lib/secure install

pgraph:
		$(MAKE) -C lib/pgraph clean
		$(MAKE) -C lib/pgraph 
		$(MAKE) -C lib/pgraph install 

build_all:	cfgparser olsrd libs
install_all:	install install_libs
