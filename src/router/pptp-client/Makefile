# $Id: Makefile,v 1.51 2011/11/29 22:05:07 quozl Exp $
VERSION=1.10.0
RELEASE=

#################################################################
# CHANGE THIS LINE to point to the location of binaries
PPPD = /usr/sbin/pppd
# Solaris
# PPPD = /usr/bin/pppd
IP = /usr/sbin/ip
#################################################################

BINDIR=$(DESTDIR)/usr/sbin
MANDIR=$(DESTDIR)/usr/share/man/man8
PPPDIR=$(DESTDIR)/etc/ppp

#CC	= gcc
RM	= rm -f
OPTIMIZE= $(COPTS) $(LTO) $(MIPS16_OPT) -ffunction-sections -fdata-sections -Wl,--gc-sections -D_GNU_SOURCE
include ../.config

#OPTIMIZE= -O0
DEBUG	= 
INCLUDE =
CFLAGS  = -Wall $(OPTIMIZE) $(DEBUG) $(INCLUDE)
LIBS	= -lutil -ffunction-sections -fdata-sections -Wl,--gc-sections
LDFLAGS	=

PPTP_BIN = pptp

PPTP_OBJS = pptp.o pptp_gre.o ppp_fcs.o \
            pptp_ctrl.o dirutil.o vector.o \
	    inststr.o util.o version.o test-redirections.o \
	    pptp_quirks.o orckit_quirks.o pqueue.o pptp_callmgr.o pptp_compat.o routing.o

PPTP_DEPS = pptp_callmgr.h pptp_gre.h ppp_fcs.h util.h test-redirections.h \
	    pptp_quirks.h orckit_quirks.h config.h pqueue.h routing.h

all: config.h $(PPTP_BIN)

$(PPTP_BIN): $(PPTP_OBJS) $(PPTP_DEPS)
	$(CC) $(COPTS) -o $(PPTP_BIN) $(PPTP_OBJS) $(LDFLAGS) $(LIBS)

config.h:
	( \
	echo "/* text added by Makefile target config.h */"; \
	echo "#define PPTP_LINUX_VERSION \"$(VERSION)$(RELEASE)\""; \
	echo "#define PPPD_BINARY \"$(PPPD)\""; \
	echo "#define IP_BINARY \"$(IP)\"" \
	) > config.h

vector_test: vector_test.o vector.o
	$(CC) -o vector_test vector_test.o vector.o
	./vector_test

clean:
	$(RM) *.o config.h pptpsetup.8

clobber: clean
	$(RM) $(PPTP_BIN) vector_test

distclean: clobber

test: vector_test

install:
	mkdir -p $(BINDIR)
	install -o root -m 555 pptp $(BINDIR)
	install -o root -m 555 pptpsetup $(BINDIR)
	mkdir -p $(MANDIR)
	install -m 644 pptp.8 $(MANDIR)
	install -m 644 pptpsetup.8 $(MANDIR)
	mkdir -p $(PPPDIR)
	install -m 644 options.pptp $(PPPDIR)

uninstall:
	$(RM) $(BINDIR)/pptp $(MANDIR)/pptp.8

dist: clobber
	$(RM) pptp-$(VERSION)$(RELEASE).tar.gz
	$(RM) -r pptp-$(VERSION)
	mkdir pptp-$(VERSION)
	cp --recursive ChangeLog Makefile *.c *.h options.pptp pptp.8 \
		pptpsetup Documentation AUTHORS COPYING INSTALL NEWS \
		README DEVELOPERS TODO USING PROTOCOL-SECURITY \
		pptp-$(VERSION)/
	$(RM) -r pptp-$(VERSION)/CVS pptp-$(VERSION)/*/CVS
	tar czf pptp-$(VERSION)$(RELEASE).tar.gz pptp-$(VERSION)
	$(RM) -r pptp-$(VERSION)
	md5sum pptp-$(VERSION)$(RELEASE).tar.gz

deb:
	chmod +x debian/rules 
	fakeroot dpkg-buildpackage -us -uc
	mv ../pptp_$(VERSION)-0_i386.deb .

WEB=~/public_html/external/mine/pptp/pptpconfig
release:
	cp pptp_$(VERSION)-0_i386.deb $(WEB)
	cd $(WEB);make

# The following include file dependencies were generated using
# "makedepend -w0 *.c", then manually removing out of tree entries.
# DO NOT DELETE

dirutil.o: dirutil.h
orckit_quirks.o: pptp_msg.h
orckit_quirks.o: pptp_compat.h
orckit_quirks.o: pptp_options.h
orckit_quirks.o: pptp_ctrl.h
orckit_quirks.o: util.h
ppp_fcs.o: ppp_fcs.h
ppp_fcs.o: pptp_compat.h
pptp.o: config.h
pptp.o: pptp_callmgr.h
pptp.o: pptp_gre.h
pptp.o: pptp_compat.h
pptp.o: version.h
pptp.o: util.h
pptp.o: pptp_quirks.h
pptp.o: pptp_msg.h
pptp.o: pptp_ctrl.h
pptp.o: pqueue.h
pptp.o: pptp_options.h
pptp_callmgr.o: pptp_callmgr.h
pptp_callmgr.o: pptp_ctrl.h
pptp_callmgr.o: pptp_compat.h
pptp_callmgr.o: pptp_msg.h
pptp_callmgr.o: dirutil.h
pptp_callmgr.o: vector.h
pptp_callmgr.o: util.h
pptp_callmgr.o: routing.h
pptp_compat.o: pptp_compat.h
pptp_compat.o: util.h
pptp_ctrl.o: pptp_msg.h
pptp_ctrl.o: pptp_compat.h
pptp_ctrl.o: pptp_ctrl.h
pptp_ctrl.o: pptp_options.h
pptp_ctrl.o: vector.h
pptp_ctrl.o: util.h
pptp_ctrl.o: pptp_quirks.h
pptp_gre.o: ppp_fcs.h
pptp_gre.o: pptp_compat.h
pptp_gre.o: pptp_msg.h
pptp_gre.o: pptp_gre.h
pptp_gre.o: util.h
pptp_gre.o: pqueue.h
pptp_gre.o: test-redirections.h
pptp_quirks.o: orckit_quirks.h
pptp_quirks.o: pptp_options.h
pptp_quirks.o: pptp_ctrl.h
pptp_quirks.o: pptp_compat.h
pptp_quirks.o: pptp_msg.h
pptp_quirks.o: pptp_quirks.h
pqueue.o: util.h
pqueue.o: pqueue.h
routing.o: routing.h
routing.o: config.h
test-redirections.o: util.h
test-redirections.o: test-redirections.h
util.o: util.h
vector.o: pptp_ctrl.h
vector.o: pptp_compat.h
vector.o: vector.h
vector_test.o: vector.h
vector_test.o: pptp_ctrl.h
vector_test.o: pptp_compat.h
version.o: config.h
