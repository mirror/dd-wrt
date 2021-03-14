
include $(TOP)/.config

ifneq ($(wildcard $(SRCBASE)/cy_conf.mak),)
  include $(SRCBASE)/cy_conf.mak
endif

all:
	install -D configs/plex.webnas $(TOP)/httpd/ej_temp/07plex.webnas
	@true

clean:
	@true

install:
	install -d $(INSTALLDIR)
	cp -rvf $(PLATFORM)/usr $(INSTALLDIR)
	rm -rf $(INSTALLDIR)/usr/share
	install -D configs/plex.nvramconfig $(INSTALLDIR)/etc/config/plex.nvramconfig
	install -D configs/plex.webnas $(INSTALLDIR)/etc/config/07plex.webnas

