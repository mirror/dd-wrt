include $(TOP)/.config

all:
ifeq ($(CONFIG_MSSID),y)
	if test -e "src/Makefile"; then make -C src; fi
	@true
endif

clean:
	if test -e "src/Makefile"; then make -C src clean; fi
	@true

install:
	if test -e "src/Makefile"; then make -C src install; else install -D nas $(INSTALLDIR)/nas/usr/sbin/nas; fi
	@true
