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
ifeq ($(CONFIG_BCM5354),y)
	if test -e "src/Makefile"; then make -C src install; else install -D nas $(INSTALLDIR)/nas/usr/sbin/nas; fi
	@true
else
	if test -e "src/Makefile"; then make -C src install; else install -D nas.vint_r10200 $(INSTALLDIR)/nas/usr/sbin/nas; fi
	@true
endif
