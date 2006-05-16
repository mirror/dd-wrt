# CryptoAPI top level Makefile

KDIR := /usr/src/linux

PACKAGE := cryptoapi
VERSION := 0.1.0

COMPILE := $(MAKE) -f Makefile.modules KDIR=$(KDIR) 

TARGETS :=

CIPHERS = $(shell ls kernel/crypto/ciphers | \
	egrep 'cipher-.*\.c$$' | sed -e 's/\.c//g' -e 's/cipher-//g')

DIGESTS = $(shell ls kernel/crypto/digests | \
	egrep 'digest-.*\.c$$' | sed -e 's/\.c//g' -e 's/digest-//g')

HEADERS = \
	crypto.h wordops.h

KERNEL_HEADERS = \
	$(patsubst %, kernel/include/linux/%, $(HEADERS))

CRYPTO_API_MODULES = \
	kernel/crypto/cryptoapi.o

CRYPTO_CIPHER_MODULES = \
	$(patsubst %,kernel/crypto/ciphers/cipher-%.o,$(CIPHERS))	

CRYPTO_DIGEST_MODULES = \
	$(patsubst %,kernel/crypto/digests/digest-%.o,$(DIGESTS)) 

CRYPTO_MODULES = \
	$(CRYPTO_API_MODULES) \
	$(CRYPTO_CIPHER_MODULES) \
	$(CRYPTO_DIGEST_MODULES)

.PHONY: default all modules patch-kernel

default:
	@echo \
"try \`make (modules or patch-kernel) KDIR=(kernelsrcdir)'"

all: default

Rules.make: $(KDIR)/Rules.make
	cp -pf $(KDIR)/Rules.make .

version Makefile.version: Rules.make arch include $(KDIR)/Makefile
	$(COMPILE) $@

KVERSION = $(shell if [ -f version ]; then cat version; fi)
KSUBLEVEL = $(shell if [ -f version ]; then cat version | cut -f 1 -d '-'; fi)
KPATCHLEVEL = \
	$(shell if [ -f version ]; then cat version | cut -f -2 -d '.'; fi)
KLOCALVERSION = $(shell uname -r)
DISTDIR := $(PACKAGE)-$(VERSION)
DISTFILES := Makefile Makefile.modules ChangeLog README RELEASE.NOTES CodingStyle cryptoapi.spec

#
MODPATH := /lib/modules/$(KVERSION)

arch: $(KDIR)/arch
	rm -f arch/
	ln -s $(KDIR)/arch .

include: $(KDIR)/include
	rm -f include/
	ln -s $(KDIR)/include .

check.stamp:
ifeq ($(KPATCHLEVEL), 2.5)
	$(error 2.5 is not supported yet, please read the README for \
	instructions on manual patching.)
else
	touch check.stamp
endif

patch-kernel: version check.stamp
	cp -R kernel/crypto $(KDIR)
	cp -R kernel/Documentation/cryptoapi $(KDIR)/Documentation
	cp $(KERNEL_HEADERS) $(KDIR)/include/linux
	cp patches/linux-$(KPATCHLEVEL)/kbuild-$(KSUBLEVEL) $(KDIR)

	mv $(KDIR)/Documentation/Configure.help \
		$(KDIR)/Documentation/Configure.help.orig
	cat $(KDIR)/Documentation/Configure.help.orig \
	    kernel/crypto/Config.help \
            kernel/crypto/ciphers/Config.help \
            kernel/crypto/digests/Config.help \
	    >>$(KDIR)/Documentation/Configure.help

	cd $(KDIR) && \
		patch -p1 <kbuild-$(KSUBLEVEL)

unpatch-kernel: version check.stamp
	rm -rf $(KDIR)/crypto $(KDIR)/Documentation/cryptoapi
	rm -rf $(patsubst %, $(KDIR)/include/linux/%, $(HEADERS))
	cd $(KDIR) && \
		patch -R -p1 <kbuild-$(KSUBLEVEL)
	mv $(KDIR)/Documentation/Configure.help.orig \
		$(KDIR)/Documentation/Configure.help
	rm -f $(KDIR)/kbuild-$(KSUBLEVEL)

modules: Rules.make arch include version check.stamp
ifneq ($(KVERSION),$(KLOCALVERSION))
		@echo "Warning: $KVERSION and uname -r differ:"
		@echo "modules are being installed in /lib/modules/$(KVERSION)"
endif
	$(COMPILE) $(CRYPTO_API_MODULES)
	$(COMPILE) $(CRYPTO_CIPHER_MODULES)
	$(COMPILE) $(CRYPTO_DIGEST_MODULES)

modules_install: $(KDIR)/Makefile modules check.stamp
	@mkdir -p --verbose $(INSTALL_MOD_PATH)$(MODPATH)/cryptoapi
	@cp -v $(CRYPTO_API_MODULES) $(INSTALL_MOD_PATH)$(MODPATH)/cryptoapi/
	@cp -v $(CRYPTO_CIPHER_MODULES) $(INSTALL_MOD_PATH)$(MODPATH)/cryptoapi/
	@cp -v $(CRYPTO_DIGEST_MODULES) $(INSTALL_MOD_PATH)$(MODPATH)/cryptoapi/

clean:
	rm -fv Makefile.version Rules.make arch include version \
		Configure.help* *.stamp
	rm -fv $(CRYPTO_MODULES)
	find -type f -name '.*.o.flags' -exec rm -fv {} \;

distclean: clean

dist: clean
	-rm -f $(DISTDIR).tar.gz	
	-rm -rf $(DISTDIR)	
	mkdir $(DISTDIR)
	cp -v $(DISTFILES) $(DISTDIR)
	cp -Rv kernel patches $(DISTDIR)
	find $(DISTDIR)/ -type d -name 'CVS' | xargs rm -rfv 
	find $(DISTDIR)/ -type f -name '*~' -exec rm -fv {} \;
	find $(DISTDIR)/ -type f -name '.#*' -exec rm -fv {} \;
	tar cvf $(DISTDIR).tar $(DISTDIR)
	gzip -9v $(DISTDIR).tar
	rm -rf $(DISTDIR)

#EOF
