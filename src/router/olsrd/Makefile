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
# $Id: Makefile,v 1.55 2005/03/31 18:39:15 kattemat Exp $

VERS =		0.4.9

CC ?= 		gcc
STRIP ?=	strip
BISON ?=	bison
FLEX ?=		flex

CCWARNINGS = -Wall -Wmissing-prototypes -Wstrict-prototypes \
             -Wmissing-declarations -Wsign-compare

INSTALL_PREFIX ?=

INCLUDES =	-Isrc

DEPFILE =	.depend

SRCS =		$(wildcard src/*.c)
HDRS =		$(wildcard src/*.h)

CFGDIR =	src/cfgparser
CFGOBJS = 	$(CFGDIR)/oscan.o $(CFGDIR)/oparse.o $(CFGDIR)/olsrd_conf.o
CFGDEPS = 	$(wildcard $(CFGDIR)/*.c) $(wildcard $(CFGDIR)/*.h) $(CFGDIR)/oparse.y $(CFGDIR)/oscan.lex

TAGCMD ?=	etags	
TAGFILE ?=	src/TAGS

ifeq ($(OS), Windows_NT)
OS =		win32
endif

ifeq ($(OS), linux)

SRCS += 	$(wildcard src/linux/*.c) $(wildcard src/unix/*.c)
HDRS +=		$(wildcard src/linux/*.h) $(wildcard src/unix/*.h)
DEFINES = 	-Dlinux
CFLAGS ?=	$(CCWARNINGS) -O2 -g #-DDEBUG #-pg #-march=i686
LIBS =		-lm -ldl #-pg
MAKEDEPEND = 	makedepend -f $(DEPFILE) $(DEFINES) -Y $(INCLUDES) $(SRCS) >/dev/null 2>&1

all:	 cfgparser olsrd
install: install_olsrd

else
ifeq ($(OS), fbsd)

SRCS +=		$(wildcard src/bsd/*.c) $(wildcard src/unix/*.c)
HDRS +=		$(wildcard src/bsd/*.h) $(wildcard src/unix/*.h)
CFLAGS ?=	$(CCWARNINGS) -O2 -g
LIBS =		-lm
MAKEDEPEND = 	makedepend -f $(DEPFILE) -D__FreeBSD__ $(INCLUDES) $(SRCS)

all:	 cfgparser olsrd
install: install_olsrd

else
ifeq ($(OS), fbsd-ll)

SRCS +=		$(wildcard src/bsd/*.c) $(wildcard src/unix/*.c)
HDRS +=		$(wildcard src/bsd/*.h) $(wildcard src/unix/*.h)
CFLAGS ?=	-Wall -Wmissing-prototypes -O2 -g -DSPOOF -I/usr/local/include
LIBS =		-lm -L/usr/local/lib -lnet
MAKEDEPEND = 	makedepend -f $(DEPFILE) -D__FreeBSD__ $(INCLUDES) $(SRCS)

all:     cfgparser olsrd
install: install_olsrd

else
ifeq ($(OS), nbsd)

SRCS +=		$(wildcard src/bsd/*.c) $(wildcard src/unix/*.c)
HDRS +=		$(wildcard src/bsd/*.h) $(wildcard src/unix/*.h)
CFLAGS ?=	$(CCWARNINGS) -O2 -g
LIBS =		-lm
MAKEDEPEND = 	makedepend -f $(DEPFILE) -D__NetBSD__ $(INCLUDES) $(SRCS)

all:	 cfgparser olsrd
install: install_olsrd

else
ifeq ($(OS), osx)

SRCS +=		$(wildcard src/bsd/*.c) $(wildcard src/unix/*.c)
HDRS +=		$(wildcard src/bsd/*.h) $(wildcard src/unix/*.h)
DEFINES =	-D__MacOSX__
CFLAGS ?=	$(CCWARNINGS) -O2 -g 
LIBS =		-lm -ldl
MAKEDEPEND = 	makedepend -f $(DEPFILE) $(DEFINES) $(INCLUDES) $(SRCS)

all:	 cfgparser olsrd
install: install_olsrd

else
ifeq ($(OS), win32)

SRCS +=		$(wildcard src/win32/*.c)
HDRS +=		$(wildcard src/win32/*.h)
INCLUDES += 	-Isrc/win32
DEFINES =	-DWIN32
CFLAGS ?=	$(CCWARNINGS) -mno-cygwin -O2 -g
LIBS =		-mno-cygwin -lws2_32 -liphlpapi
MAKEDEPEND = 	makedepend -f $(DEPFILE) $(DEFINES) $(INCLUDES) $(SRCS)

all:	 cfgparser olsrd
install: install_olsrd

olsr-${VERS}.zip:	gui/win32/Main/Release/Switch.exe \
		gui/win32/Shim/Release/Shim.exe \
		olsrd.exe \
		src/cfgparser/olsrd_cfgparser.dll \
		README \
		README-Link-Quality.html \
		gui/win32/Inst/linux-manual.txt \
		files/olsrd.conf.win32.rfc \
		files/olsrd.conf.win32.lq \
		gui/win32/Main/RFC-Default.olsr \
		gui/win32/Main/LQ-Default.olsr \
		lib/dot_draw/olsrd_dot_draw.dll \
		lib/nameservice/olsrd_nameservice.dll \
		lib/httpinfo/olsrd_httpinfo.dll
		$(STRIP) olsrd.exe
		$(STRIP) src/cfgparser/olsrd_cfgparser.dll
		$(STRIP) lib/dot_draw/olsrd_dot_draw.dll
		$(STRIP) lib/nameservice/olsrd_nameservice.dll
		$(STRIP) lib/httpinfo/olsrd_httpinfo.dll
		rm -rf ${TEMP}/olsr-${VERS}
		rm -f ${TEMP}/olsr-${VERS}.zip
		rm -f olsr-${VERS}.zip
		mkdir ${TEMP}/olsr-${VERS}
		cp gui/win32/Main/Release/Switch.exe ${TEMP}/olsr-${VERS}
		cp gui/win32/Shim/Release/Shim.exe ${TEMP}/olsr-${VERS}
		cp olsrd.exe ${TEMP}/olsr-${VERS}
		cp src/cfgparser/olsrd_cfgparser.dll ${TEMP}/olsr-${VERS}
		cp README ${TEMP}/olsr-${VERS}
		cp README-Link-Quality.html ${TEMP}/olsr-${VERS}
		cp gui/win32/Inst/linux-manual.txt ${TEMP}/olsr-${VERS}
		cp files/olsrd.conf.win32.rfc ${TEMP}/olsr-${VERS}/olsrd.conf.rfc
		cp files/olsrd.conf.win32.lq ${TEMP}/olsr-${VERS}/olsrd.conf.lq
		cp gui/win32/Main/RFC-Default.olsr ${TEMP}/olsr-${VERS}
		cp gui/win32/Main/LQ-Default.olsr ${TEMP}/olsr-${VERS}/Default.olsr
		cp lib/dot_draw/olsrd_dot_draw.dll ${TEMP}/olsr-${VERS}
		cp lib/nameservice/olsrd_nameservice.dll ${TEMP}/olsr-${VERS}
		cp lib/httpinfo/olsrd_httpinfo.dll ${TEMP}/olsr-${VERS}
		cd ${TEMP}; echo y | cacls olsr-${VERS} /T /G Everyone:F
		cd ${TEMP}; zip -q -r olsr-${VERS}.zip olsr-${VERS}
		cp ${TEMP}/olsr-${VERS}.zip .
		rm -rf ${TEMP}/olsr-${VERS}
		rm -f ${TEMP}/olsr-${VERS}.zip

olsr-${VERS}-setup.exe:	gui/win32/Main/Release/Switch.exe \
		gui/win32/Shim/Release/Shim.exe \
		olsrd.exe \
		src/cfgparser/olsrd_cfgparser.dll \
		README \
		README-Link-Quality.html \
		gui/win32/Inst/linux-manual.txt \
		files/olsrd.conf.win32.rfc \
		files/olsrd.conf.win32.lq \
		gui/win32/Main/RFC-Default.olsr \
		gui/win32/Main/LQ-Default.olsr \
		lib/dot_draw/olsrd_dot_draw.dll \
		lib/nameservice/olsrd_nameservice.dll \
		lib/httpinfo/olsrd_httpinfo.dll \
		gui/win32/Inst/installer.nsi
		$(STRIP) olsrd.exe
		$(STRIP) src/cfgparser/olsrd_cfgparser.dll
		$(STRIP) lib/dot_draw/olsrd_dot_draw.dll
		$(STRIP) lib/nameservice/olsrd_nameservice.dll
		$(STRIP) lib/httpinfo/olsrd_httpinfo.dll
		rm -f olsr-setup.exe
		rm -f olsr-${VERS}-setup.exe
		C:/Program\ Files/NSIS/makensis gui\win32\Inst\installer.nsi
		mv olsr-setup.exe olsr-${VERS}-setup.exe

else
ifeq ($(OS), wince)

SRCS +=		$(wildcard src/win32/*.c)
HDRS +=		$(wildcard src/win32/*.h)
INCLUDES += 	-Isrc/win32 -Isrc/win32/ce
DEFINES =	-DWIN32 -DWINCE
CFLAGS ?=	$(CCWARNINGS) -O2 -g
LIBS =		-lwinsock -liphlpapi
MAKEDEPEND = 	makedepend -f $(DEPFILE) $(DEFINES) $(INCLUDES) $(SRCS)

else

all:	help
install:help
endif
endif
endif
endif
endif
endif
endif

ifneq ($(NODEBUG), )
CFLAGS += -DNODEBUG
endif

OBJS = $(patsubst %.c,%.o,$(SRCS))
override CFLAGS += $(INCLUDES) $(DEFINES)
export CFLAGS


olsrd:		$(OBJS) $(CFGOBJS)
		$(CC) -o $@ $(OBJS) $(CFGOBJS) $(LIBS) 

$(DEPFILE):	$(SRCS) $(HDRS)
ifdef MAKEDEPEND
		@echo '# olsrd dependency file. AUTOGENERATED' > $(DEPFILE)
		$(MAKEDEPEND)
endif

cfgparser:	$(CFGDEPS)
		$(MAKE) -C $(CFGDIR)

$(CFGOBJS):
		$(MAKE) -C $(CFGDIR)

.PHONY: help libs clean_libs clean uberclean install_libs install_bin install

help:
	@echo
	@echo '***** olsr.org olsr daemon Make ****'
	@echo ' You must provide a valid target OS '
	@echo ' by setting the OS variable! Valid  '
	@echo ' target OSes are:                   '
	@echo ' ---------------------------------  '
	@echo ' linux - GNU/Linux                  '
	@echo ' win32 - MS Windows                 '
	@echo ' fbsd  - FreeBSD                    '
	@echo ' nbsd  - NetBSD                     '
	@echo ' osx   - Mac OS X                   '
	@echo ' ---------------------------------  '
	@echo ' Example - build for windows:       '
	@echo ' make OS=win32                      '
	@echo ' If you are developing olsrd code,  '
	@echo ' exporting the OS variable might    '
	@echo ' be a good idea :-) Have fun!       '
	@echo '************************************'
	@echo

clean:
		rm -f $(OBJS) olsrd olsrd.exe
		$(MAKE) -C src/cfgparser clean

uberclean:	clean clean_libs
		rm -f $(DEPFILE) $(DEPFILE).bak *~
		rm -f src/*[o~] src/linux/*[o~] src/unix/*[o~] src/win32/*[o~]
		rm -f src/bsd/*[o~] 
		$(MAKE) -C src/cfgparser uberclean

install_bin:
		$(STRIP) olsrd
		mkdir -p $(INSTALL_PREFIX)/usr/sbin
		install -m 755 olsrd $(INSTALL_PREFIX)/usr/sbin

install_olsrd:	install_bin
		@echo ========= C O N F I G U R A T I O N - F I L E ============
		@echo olsrd uses the configfile $(INSTALL_PREFIX)/etc/olsr.conf
		@echo a default configfile. A sample RFC-compliance aimed
		@echo configfile can be installed. Note that a LQ-based configfile
		@echo can be found at files/olsrd.conf.default.lq
		@echo ==========================================================
		mkdir -p $(INSTALL_PREFIX)/etc
		cp -i files/olsrd.conf.default.rfc $(INSTALL_PREFIX)/etc/olsrd.conf
		@echo -------------------------------------------
		@echo Edit $(INSTALL_PREFIX)/etc/olsrd.conf before running olsrd!!
		@echo -------------------------------------------
		@echo Installing manpages olsrd\(8\) and olsrd.conf\(5\)
		mkdir -p $(INSTALL_PREFIX)/usr/share/man/man8/
		cp files/olsrd.8.gz $(INSTALL_PREFIX)/usr/share/man/man8/olsrd.8.gz
		mkdir -p $(INSTALL_PREFIX)/usr/share/man/man5/
		cp files/olsrd.conf.5.gz $(INSTALL_PREFIX)/usr/share/man/man5/olsrd.conf.5.gz

#
# PLUGINS
#

libs: 
		for i in lib/*; do \
			$(MAKE) -C $$i; \
		done; 

clean_libs: 
		for i in lib/*; do \
			$(MAKE) -C $$i clean; \
		done; 

install_libs:
		for i in lib/*; do \
			$(MAKE) -C $$i LIBDIR=$(INSTALL_PREFIX)/usr/lib install; \
		done; 	

httpinfo:
		$(MAKE) -C lib/httpinfo clean
		$(MAKE) -C lib/httpinfo 
		$(MAKE) -C lib/httpinfo install 

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

powerinfo:
		$(MAKE) -C lib/powerinfo clean
		$(MAKE) -C lib/powerinfo 
		$(MAKE) -C lib/powerinfo install

secure:
		$(MAKE) -C lib/secure clean
		$(MAKE) -C lib/secure
		$(MAKE) -C lib/secure install


tags:
		$(TAGCMD) -o $(TAGFILE) $(SRCS) $(HDRS) $(wildcard src/cfgparser/*.c) $(wildcard src/cfgparser/*.h)

sinclude	$(DEPFILE)
