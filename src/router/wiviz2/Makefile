# $Id: Makefile 1006 2005-05-21 12:04:17Z nico $

include $(TOPDIR)/rules.mk

PKG_NAME:=wiviz
PKG_VERSION:=2.0
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

WIVIZ_SOURCES=wiviz.c wl_access.c channelhopper.c

include $(TOPDIR)/package/rules.mk

$(eval $(call PKG_template,WIVIZ,wiviz,$(PKG_VERSION)-$(PKG_RELEASE),$(ARCH)))

$(PKG_BUILD_DIR)/.configured:
	touch $@
	
$(PKG_BUILD_DIR)/.prepared:
	mkdir -p $@
	touch $@

$(PKG_BUILD_DIR)/.built: $(WIVIZ_SOURCES)
	$(TARGET_CC) $(TARGET_CFLAGS) -o $(PKG_BUILD_DIR)/wiviz2 $(WIVIZ_SOURCES)
	touch $@

$(IPKG_WIVIZ): $(PKG_BUILD_DIR)/.built
	mkdir -p $(IDIR_WIVIZ)/usr/sbin
	cp $(PKG_BUILD_DIR)/wiviz2 $(IDIR_WIVIZ)/usr/sbin/
	$(RSTRIP) $(IDIR_WIVIZ)/usr/sbin/wiviz2
	mkdir -p $(IDIR_WIVIZ)/www/wiviz2
	mkdir -p $(IDIR_WIVIZ)/www/cgi-bin
	cp -R www/* $(IDIR_WIVIZ)/www/
	chmod 755 $(IDIR_WIVIZ)/www/cgi-bin/*.cgi
	chmod 755 $(IDIR_WIVIZ)/usr/sbin/wiviz2
	$(IPKG_BUILD) $(IDIR_WIVIZ) $(PACKAGE_DIR)

all: $(IPKG_WIVIZ)

mostlyclean:
	rm -f $(PKG_BUILD_DIR)/wiviz2
	rm -f $(IPKG_WIVIZ)
	rm -f $(PKG_BUILD_DIR)/.built
	
