
include $(TOP)/.config

ifneq ($(wildcard $(SRCBASE)/cy_conf.mak),)
  include $(SRCBASE)/cy_conf.mak
endif

MODULE:=language

all:
	cp $(MODULE)* $(INSTALLDIR)/etc/config

clean:
	@true

install:
	install -d $(INSTALLDIR)/etc
	install -d $(INSTALLDIR)/etc/config

