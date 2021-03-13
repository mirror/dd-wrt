
include $(TOP)/.config

ifneq ($(wildcard $(SRCBASE)/cy_conf.mak),)
  include $(SRCBASE)/cy_conf.mak
endif

all:
	@true

clean:
	@true

install:
	install -d $(INSTALLDIR)
	cp -rvf $(PLATFORM)/usr $(INSTALLDIR)
	rm -rf $(INSTALLDIR)/usr/share

