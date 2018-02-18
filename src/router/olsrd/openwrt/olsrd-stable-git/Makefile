# 
# Copyright (C) 2009-2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=olsrd-stable-git
PKG_VERSION:=$(shell date +%Y%m%d)+$(shell git --git-dir=$(CURDIR)/../../.git log -1 --pretty=%h)
PKG_RELEASE:=1

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

TARGET_CFLAGS += $(FPIC)

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) $(CURDIR)/../../* $(PKG_BUILD_DIR)/
endef

define Package/olsrd-stable-git/template
  SECTION:=net
  CATEGORY:=Network
  MAINTAINER:=OLSR Development Team <olsr-dev@lists.olsr.org>
  TITLE:=OLSR (Optimized Link State Routing) daemon - Stable Branch
  URL:=http://www.olsr.org/
endef

define Package/olsrd-stable-git
  $(call Package/olsrd-stable-git/template)
  MENU:=1
  DEPENDS:=+libpthread
endef

define Package/olsrd-stable-git/conffiles
/etc/config/olsrd
endef

define Package/olsrd-stable-git-mod-arprefresh
  $(call Package/olsrd-stable-git/template)
  DEPENDS:=olsrd-stable-git
  TITLE:=Kernel ARP cache refresh plugin
endef

define Package/olsrd-stable-git-mod-dot-draw
  $(call Package/olsrd-stable-git/template)
  DEPENDS:=olsrd-stable-git
  TITLE:=Dot topology information plugin
endef

define Package/olsrd-stable-git-mod-bmf
  $(call Package/olsrd-stable-git/template)
  DEPENDS:=olsrd-stable-git +kmod-tun
  TITLE:=Basic multicast forwarding plugin
endef

define Package/olsrd-stable-git-mod-dyn-gw
  $(call Package/olsrd-stable-git/template)
  DEPENDS:=olsrd-stable-git
  TITLE:=Dynamic internet gateway plugin
endef

define Package/olsrd-stable-git-mod-dyn-gw-plain
  $(call Package/olsrd-stable-git/template)
  DEPENDS:=olsrd-stable-git
  TITLE:=Dynamic internet gateway plain plugin
endef

define Package/olsrd-stable-git-mod-httpinfo
  $(call Package/olsrd-stable-git/template)
  DEPENDS:=olsrd-stable-git
  TITLE:=Small informative web server plugin
endef

define Package/olsrd-stable-git-mod-jsoninfo
  $(call Package/olsrd-stable-git/template)
  DEPENDS:=olsrd-stable-git
  TITLE:=json output information plugin
endef

define Package/olsrd-stable-git-mod-mdns
  $(call Package/olsrd-stable-git/template)
  DEPENDS:=olsrd-stable-git
  TITLE:=Multicast DNS plugin
endef

define Package/olsrd-stable-git-mod-nameservice
  $(call Package/olsrd-stable-git/template)
  DEPENDS:=olsrd-stable-git
  TITLE:=Lightweight hostname resolver plugin
endef

define Package/olsrd-stable-git-mod-netjson
  $(call Package/olsrd-stable-git/template)
  DEPENDS:=olsrd-stable-git
  TITLE:=netjson output information plugin
endef

define Package/olsrd-stable-git-mod-p2pd
  $(call Package/olsrd-stable-git/template)
  DEPENDS:=olsrd-stable-git
  TITLE:=Peer to Peer Discovery plugin
endef

define Package/olsrd-stable-git-mod-quagga
  $(call Package/olsrd-stable-git/template)
  DEPENDS:=olsrd-stable-git
  TITLE:=Quagga plugin
endef

define Package/olsrd-stable-git-mod-secure
  $(call Package/olsrd-stable-git/template)
  DEPENDS:=olsrd-stable-git
  TITLE:=Message signing plugin to secure routing domain
endef

define Package/olsrd-stable-git-mod-txtinfo
  $(call Package/olsrd-stable-git/template)
  DEPENDS:=olsrd-stable-git
  TITLE:=Small informative web server plugin
endef

define Package/olsrd-stable-git-mod-watchdog
  $(call Package/olsrd-stable-git/template)
  DEPENDS:=olsrd-stable-git
  TITLE:=Watchdog plugin
endef

define Package/olsrd-stable-git-mod-secure/conffiles
/etc/olsrd.d/olsrd_secure_key
endef

MAKE_FLAGS+= \
	NO_DEBUG_MESSAGES=1 \
	OS="linux" \
	DESTDIR="$(PKG_INSTALL_DIR)" \
	STRIP="true" \
	INSTALL_LIB="true" \
	SUBDIRS="arprefresh bmf dot_draw dyn_gw dyn_gw_plain httpinfo jsoninfo mdns nameservice netjson quagga secure txtinfo watchdog"

define Build/Compile
	$(call Build/Compile/Default,clean)
	$(call Build/Compile/Default,clean_libs)
	$(call Build/Compile/Default,all)
	$(call Build/Compile/Default,libs)
endef

define Build/Install
	$(call Build/Install/Default,install)
	$(call Build/Install/Default,install_libs)
endef

define Package/olsrd-stable-git/install
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DATA) ./files/olsrd.config $(1)/etc/config/olsrd
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/olsrd $(1)/usr/sbin/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/olsrd.init $(1)/etc/init.d/olsrd
endef

define Package/olsrd-stable-git-mod-arprefresh/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/lib/arprefresh/olsrd_arprefresh.so.* $(1)/usr/lib/
endef

define Package/olsrd-stable-git-mod-dot-draw/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/lib/dot_draw/olsrd_dot_draw.so.* $(1)/usr/lib/
endef

define Package/olsrd-stable-git-mod-bmf/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/lib/bmf/olsrd_bmf.so.* $(1)/usr/lib/
endef

define Package/olsrd-stable-git-mod-dyn-gw/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/lib/dyn_gw/olsrd_dyn_gw.so.* $(1)/usr/lib/
endef

define Package/olsrd-stable-git-mod-dyn-gw-plain/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/lib/dyn_gw_plain/olsrd_dyn_gw_plain.so.* $(1)/usr/lib/
endef

define Package/olsrd-stable-git-mod-httpinfo/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/lib/httpinfo/olsrd_httpinfo.so.* $(1)/usr/lib/
endef

define Package/olsrd-stable-git-mod-jsoninfo/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/lib/jsoninfo/olsrd_jsoninfo.so.* $(1)/usr/lib/
endef

define Package/olsrd-stable-git-mod-mdns/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/lib/mdns/olsrd_mdns.so.* $(1)/usr/lib/
endef

define Package/olsrd-stable-git-mod-nameservice/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/lib/nameservice/olsrd_nameservice.so.* $(1)/usr/lib/
endef

define Package/olsrd-stable-git-mod-netjson/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/lib/netjson/olsrd_netjson.so.* $(1)/usr/lib/
endef

define Package/olsrd-stable-git-mod-quagga/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/lib/quagga/olsrd_quagga.so.* $(1)/usr/lib/
endef

define Package/olsrd-stable-git-mod-secure/install
	$(INSTALL_DIR) $(1)/etc/olsrd.d
	$(CP) ./files/olsrd_secure_key $(1)/etc/olsrd.d/
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/lib/secure/olsrd_secure.so.* $(1)/usr/lib/
endef

define Package/olsrd-stable-git-mod-txtinfo/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/lib/txtinfo/olsrd_txtinfo.so.* $(1)/usr/lib/
endef

define Package/olsrd-stable-git-mod-watchdog/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/lib/watchdog/olsrd_watchdog.so.* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,olsrd-stable-git))
$(eval $(call BuildPackage,olsrd-stable-git-mod-arprefresh))
$(eval $(call BuildPackage,olsrd-stable-git-mod-dot-draw))
$(eval $(call BuildPackage,olsrd-stable-git-mod-bmf))
$(eval $(call BuildPackage,olsrd-stable-git-mod-dyn-gw))
$(eval $(call BuildPackage,olsrd-stable-git-mod-dyn-gw-plain))
$(eval $(call BuildPackage,olsrd-stable-git-mod-httpinfo))
$(eval $(call BuildPackage,olsrd-stable-git-mod-jsoninfo))
$(eval $(call BuildPackage,olsrd-stable-git-mod-mdns))
$(eval $(call BuildPackage,olsrd-stable-git-mod-nameservice))
$(eval $(call BuildPackage,olsrd-stable-git-mod-netjson))
$(eval $(call BuildPackage,olsrd-stable-git-mod-quagga))
$(eval $(call BuildPackage,olsrd-stable-git-mod-secure))
$(eval $(call BuildPackage,olsrd-stable-git-mod-txtinfo))
$(eval $(call BuildPackage,olsrd-stable-git-mod-watchdog))
