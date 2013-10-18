include $(TOP)/.config

all:
ifeq ($(CONFIG_MSSID),y)
	if test -e "src/Makefile"; then make -C src; fi
endif
	@true

clean:
	if test -e "src/Makefile"; then make -C src clean; fi
	@true

install:
ifneq ($(CONFIG_BCMMODERN),y)
	if test -e "src/Makefile"; then make -C src install; else install -D nas $(INSTALLDIR)/nas/usr/sbin/nas; fi
	@true
else
ifeq ($(PLATFORM),arm)
	if test -e "src/Makefile"; then make -C src install; else install -D nasmodern.arm $(INSTALLDIR)/nas/usr/sbin/nas; fi
else
	if test -e "src/Makefile"; then make -C src install; else install -D nasmodern $(INSTALLDIR)/nas/usr/sbin/nas; fi
endif
	@true
endif
