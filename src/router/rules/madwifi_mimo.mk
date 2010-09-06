madwifi_mimo-checkout:
	rm -rf $(TOP)/madwifi.dev/madwifi_mimo.dev
	svn co svn://svn.dd-wrt.com/private/madwifi_mimo $(TOP)/madwifi.dev/madwifi_mimo.dev

madwifi_mimo-update:
	svn update $(TOP)/madwifi.dev/madwifi_mimo.dev

ifeq ($(CONFIG_AHB),y)
madwifi_mimo:
	make -C madwifi.dev/madwifi_mimo.dev/core/hal/linux TARGET=$(ARCHITECTURE)-elf AR9100=1 AR5416_G_MODE=1 GSOAP_WITH_LEANER=1
	make -C madwifi.dev/madwifi_mimo.dev KERNELPATH=$(LINUXDIR) TARGET=$(ARCHITECTURE)-elf  AR9100=1 AR5416_G_MODE=1 GSOAP_WITH_LEANER=1 BUS=AHB
	make -C madwifi.dev/madwifi_mimo.dev/tools KERNELPATH=$(LINUXDIR) TARGET=$(ARCHITECTURE)-elf BINDIR=$(INSTALLDIR)/madwifi/usr/sbin  AR9100=1 AR5416_G_MODE=1 GSOAP_WITH_LEANER=1 BUS=AHB

madwifi_mimo-clean:
	make -C madwifi.dev/madwifi_mimo.dev/core/hal/linux TARGET=$(ARCHITECTURE)-elf clean
	make -C madwifi.dev/madwifi_mimo.dev clean KERNELPATH=$(LINUXDIR) TARGET=$(ARCHITECTURE)-elf
	make -C madwifi.dev/madwifi_mimo.dev/tools KERNELPATH=$(LINUXDIR) BINDIR=$(INSTALLDIR)/madwifi/usr/sbin clean

madwifi_mimo-install:
	mkdir -p $(INSTALLDIR)/madwifi/usr/sbin
	make -C madwifi.dev/madwifi_mimo.dev/tools KERNELPATH=$(LINUXDIR) BINDIR=/madwifi/usr/sbin DESTDIR=$(INSTALLDIR) install   AR9100=1 AR5416_G_MODE=1 GSOAP_WITH_LEANER=1 BUS=AHB
	make -C madwifi.dev/madwifi_mimo.dev KERNELPATH=$(LINUXDIR) BINDIR=/usr/sbin DESTDIR=$(INSTALLDIR)/madwifi MODULEPATH=/lib/80211n  TARGET=mipsisa32-be-elf install   AR9100=1 AR5416_G_MODE=1 GSOAP_WITH_LEANER=1 BUS=AHB
else
madwifi_mimo:
	make -C madwifi.dev/madwifi_mimo.dev/core/hal/linux TARGET=$(ARCHITECTURE)-elf AR5416_G_MODE=1 GSOAP_WITH_LEANER=1
	make -C madwifi.dev/madwifi_mimo.dev KERNELPATH=$(LINUXDIR) TARGET=$(ARCHITECTURE)-elf AR5416_G_MODE=1 GSOAP_WITH_LEANER=1
	make -C madwifi.dev/madwifi_mimo.dev/tools KERNELPATH=$(LINUXDIR) TARGET=$(ARCHITECTURE)-elf BINDIR=$(INSTALLDIR)/madwifi/usr/sbin AR5416_G_MODE=1 GSOAP_WITH_LEANER=1

madwifi_mimo-clean:
	make -C madwifi.dev/madwifi_mimo.dev/core/hal/linux TARGET=$(ARCHITECTURE)-elf clean
	make -C madwifi.dev/madwifi_mimo.dev clean KERNELPATH=$(LINUXDIR) TARGET=$(ARCHITECTURE)-elf
	make -C madwifi.dev/madwifi_mimo.dev/tools KERNELPATH=$(LINUXDIR) BINDIR=$(INSTALLDIR)/madwifi/usr/sbin clean

madwifi_mimo-install:
	mkdir -p $(INSTALLDIR)/madwifi/usr/sbin
	make -C madwifi.dev/madwifi_mimo.dev/tools KERNELPATH=$(LINUXDIR) BINDIR=/madwifi/usr/sbin DESTDIR=$(INSTALLDIR) install AR5416_G_MODE=1 GSOAP_WITH_LEANER=1
	make -C madwifi.dev/madwifi_mimo.dev KERNELPATH=$(LINUXDIR) BINDIR=/usr/sbin DESTDIR=$(INSTALLDIR)/madwifi MODULEPATH=/lib/80211n  TARGET=$(ARCHITECTURE)-elf install AR5416_G_MODE=1 GSOAP_WITH_LEANER=1
endif