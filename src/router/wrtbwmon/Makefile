
include $(TOP)/.config

ifneq ($(wildcard $(SRCBASE)/cy_conf.mak),)
  include $(SRCBASE)/cy_conf.mak
endif

clean:
	@true
all:
	@true

install:
	install -d $(INSTALLDIR)/usr/share/wrtbwmon
	install -d $(INSTALLDIR)/usr/sbin
ifeq ($(CONFIG_NFTABLES),y)
	cp wrtbwmon.nft $(INSTALLDIR)/usr/sbin/wrtbwmon
	cp readDB.awk.nft $(INSTALLDIR)/usr/sbin/readDB.awk
	cp usage.htm1 usage.htm2 $(INSTALLDIR)/usr/share/wrtbwmon
else
	cp wrtbwmon readDB.awk $(INSTALLDIR)/usr/sbin
	cp usage.htm1 usage.htm2 $(INSTALLDIR)/usr/share/wrtbwmon
endif
