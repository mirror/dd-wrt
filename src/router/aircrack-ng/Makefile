AC_ROOT		= .
include		$(AC_ROOT)/common.mak

SCRIPTS         = scan scantidy
DOCFILES        = ChangeLog INSTALLING README LICENSE AUTHORS VERSION

COVERITY_BUILD	?= cov-build
COVERITY_DIR	= cov-int

COVERITY_TAR_GZ	= Aircrack-ng.tar.gz
COVERITY_CREDS_DIR	= coverity
COVERITY_TOKEN	= $(shell cat ${COVERITY_CREDS_DIR}/token)
COVERITY_EMAIL	= $(shell cat ${COVERITY_CREDS_DIR}/email)

default: all

all:
	$(MAKE) -C src $(@)

coverity-build:
	$(COVERITY_BUILD) --dir $(COVERITY_DIR) $(MAKE) sqlite=true experimental=true pcre=true

coverity-package: coverity-build
	tar czvf $(COVERITY_TAR_GZ) $(COVERITY_DIR)

coverity-upload: coverity-package
	curl --form project=Aircrack-ng --form token=$(COVERITY_TOKEN) --form email=$(COVERITY_EMAIL) --form file=@$(COVERITY_TAR_GZ) --form version=r$(REVISION) --form description="Aircrack-ng svn r$(REVISION)" http://scan5.coverity.com/cgi-bin/upload.py

coverity-show-creds:
	@echo "Token: $(COVERITY_TOKEN)"
	@echo "Email: $(COVERITY_EMAIL)"

aircrack-ng-opt-prof_gen: all
	mkdir -p prof
	$(MAKE) -C src $(@)

aircrack-ng-opt-prof_use:
	$(MAKE) -C src $(@)

install: all
	$(MAKE) -C src $(@)
	$(MAKE) -C scripts $(@)
	install -m 755 $(SCRIPTS) $(sbindir)

uninstall:
	$(MAKE) -C src $(@)
	-rm -f $(DESTDIR)$(sbindir)/airmon-ng
	-rm -f $(DESTDIR)$(sbindir)/airdriver-ng
	-rm -f $(DESTDIR)$(sbindir)/patchchk
	-rm -f $(DESTDIR)$(mandir)/aircrack-ng.1
	-rm -f $(DESTDIR)$(mandir)/airdecap-ng.1
	-rm -f $(DESTDIR)$(mandir)/airdriver-ng.1
	-rm -f $(DESTDIR)$(mandir)/aireplay-ng.1
	-rm -f $(DESTDIR)$(mandir)/airmon-ng.1
	-rm -f $(DESTDIR)$(mandir)/airodump-ng.1
	-rm -f $(DESTDIR)$(mandir)/airolib-ng.1
	-rm -f $(DESTDIR)$(mandir)/airsev-ng.1
	-rm -f $(DESTDIR)$(mandir)/airtun-ng.1
	-rm -f $(DESTDIR)$(mandir)/buddy-ng.1
	-rm -f $(DESTDIR)$(mandir)/easside-ng.1
	-rm -f $(DESTDIR)$(mandir)/ivstools.1
	-rm -f $(DESTDIR)$(mandir)/kstats.1
	-rm -f $(DESTDIR)$(mandir)/makeivs-ng.1
	-rm -f $(DESTDIR)$(mandir)/packetforge-ng.1
	-rm -f $(DESTDIR)$(mandir)/wesside-ng.1
	-rm -fr $(DESTDIR)$(docdir)

strip:
	$(MAKE) -C src $(@)

doc:
	install -d $(DESTDIR)$(docdir)
	install -m 644 $(DOCFILES) $(DESTDIR)$(docdir)

clean:
	$(MAKE) -C src $(@)

distclean: clean

