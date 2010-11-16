
ifeq ($(CONFIG_BUFFALO),y)
export EXTRA_VERSION=-buffalo
endif


ifeq ($(CONFIG_AHB),y)
madwifi_mimo:
	make -C madwifi.dev/madwifi_mimo.dev/core/hal/linux TARGET=$(ARCHITECTURE)$(EXTRA_VERSION)-elf AR9100=1 AR5416_G_MODE=1 GSOAP_WITH_LEANER=1
	make -C madwifi.dev/madwifi_mimo.dev KERNELPATH=$(LINUXDIR) TARGET=$(ARCHITECTURE)$(EXTRA_VERSION)-elf  AR9100=1 AR5416_G_MODE=1 GSOAP_WITH_LEANER=1 BUS=AHB
	make -C madwifi.dev/madwifi_mimo.dev/tools KERNELPATH=$(LINUXDIR) TARGET=$(ARCHITECTURE)$(EXTRA_VERSION)-elf BINDIR=$(INSTALLDIR)/madwifi/usr/sbin  AR9100=1 AR5416_G_MODE=1 GSOAP_WITH_LEANER=1 BUS=AHB

ifeq ($(CONFIG_MADWIFI),y)
madwifi_mimo-clean:
	make -C madwifi.dev/madwifi_mimo.dev/core/hal/linux TARGET=$(ARCHITECTURE)$(EXTRA_VERSION)-elf clean
	make -C madwifi.dev/madwifi_mimo.dev clean KERNELPATH=$(LINUXDIR) TARGET=$(ARCHITECTURE)$(EXTRA_VERSION)-elf
	make -C madwifi.dev/madwifi_mimo.dev/tools KERNELPATH=$(LINUXDIR) BINDIR=$(INSTALLDIR)/madwifi/usr/sbin clean
else
	@true
endif	

madwifi_mimo-install:
	mkdir -p $(INSTALLDIR)/madwifi/usr/sbin
	make -C madwifi.dev/madwifi_mimo.dev/tools KERNELPATH=$(LINUXDIR) BINDIR=/madwifi/usr/sbin DESTDIR=$(INSTALLDIR) install   AR9100=1 AR5416_G_MODE=1 GSOAP_WITH_LEANER=1 BUS=AHB
	make -C madwifi.dev/madwifi_mimo.dev KERNELPATH=$(LINUXDIR) BINDIR=/usr/sbin DESTDIR=$(INSTALLDIR)/madwifi MODULEPATH=/lib/80211n  TARGET=$(ARCHITECTURE)$(EXTRA_VERSION)-elf install   AR9100=1 AR5416_G_MODE=1 GSOAP_WITH_LEANER=1 BUS=AHB
else
madwifi_mimo:
	make -C madwifi.dev/madwifi_mimo.dev/core/hal/linux TARGET=$(ARCHITECTURE)$(EXTRA_VERSION)-elf AR5416_G_MODE=1 GSOAP_WITH_LEANER=1
	make -C madwifi.dev/madwifi_mimo.dev KERNELPATH=$(LINUXDIR) TARGET=$(ARCHITECTURE)$(EXTRA_VERSION)-elf AR5416_G_MODE=1 GSOAP_WITH_LEANER=1
	make -C madwifi.dev/madwifi_mimo.dev/tools KERNELPATH=$(LINUXDIR) TARGET=$(ARCHITECTURE)$(EXTRA_VERSION)-elf BINDIR=$(INSTALLDIR)/madwifi/usr/sbin AR5416_G_MODE=1 GSOAP_WITH_LEANER=1

ifeq ($(CONFIG_MADWIFI),y)	
madwifi_mimo-clean:
	make -C madwifi.dev/madwifi_mimo.dev/core/hal/linux TARGET=$(ARCHITECTURE)$(EXTRA_VERSION)-elf clean
	make -C madwifi.dev/madwifi_mimo.dev clean KERNELPATH=$(LINUXDIR) TARGET=$(ARCHITECTURE)$(EXTRA_VERSION)-elf
	make -C madwifi.dev/madwifi_mimo.dev/tools KERNELPATH=$(LINUXDIR) BINDIR=$(INSTALLDIR)/madwifi/usr/sbin clean
else
	@true
endif

madwifi_mimo-install:
	mkdir -p $(INSTALLDIR)/madwifi/usr/sbin
	make -C madwifi.dev/madwifi_mimo.dev/tools KERNELPATH=$(LINUXDIR) BINDIR=/madwifi/usr/sbin DESTDIR=$(INSTALLDIR) install AR5416_G_MODE=1 GSOAP_WITH_LEANER=1
	make -C madwifi.dev/madwifi_mimo.dev KERNELPATH=$(LINUXDIR) BINDIR=/usr/sbin DESTDIR=$(INSTALLDIR)/madwifi MODULEPATH=/lib/80211n  TARGET=$(ARCHITECTURE)$(EXTRA_VERSION)-elf install AR5416_G_MODE=1 GSOAP_WITH_LEANER=1
endif