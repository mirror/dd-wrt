# ====================================================================
#
#   Copyright (C) 2013 Qualcomm Atheros, Inc.
#   
#   Permission to use, copy, modify, and/or distribute this software 
#   for any purpose with or without fee is hereby granted, provided 
#   that the above copyright notice and this permission notice appear 
#   in all copies.
#   
#   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL 
#   WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED 
#   WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL  
#   THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR 
#   CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM 
#   LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, 
#   NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
#   CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
#   
# --------------------------------------------------------------------

include $(TOPDIR)/rules.mk

# ====================================================================
# package identity constants;
# --------------------------------------------------------------------

PKG_NAME:=open-plc-utils
PKG_VERSION:=2013-01-29
PKG_RELEASE:=6b4b5543cc2c961178260695f7a49f7a50308f36

# ====================================================================
# package download constants;
# --------------------------------------------------------------------

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=git://github.com/qca/open-plc-utils.git
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_VERSION:=$(PKG_RELEASE)
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz

# ====================================================================
# package build constants;
# --------------------------------------------------------------------

# PKG_BUILD_PARALLEL:=1

# ====================================================================
#
# --------------------------------------------------------------------

define Package/open-plc-utils/default
 SECTION:=utils
 CATEGORY:=Utilities
 TITLE:=Qualcomm Atheros Powerline Communication utilities
 URL:=https://github.qualcomm.com/cmaier/open-plc-utils/blob/master/README
 MAINTAINER:=Qualcomm Atheros, Ocala FL USA
endef

# ====================================================================
#
# --------------------------------------------------------------------

include $(INCLUDE_DIR)/package.mk

# ====================================================================
#
# --------------------------------------------------------------------

define Package/open-plc-utils
 $(call Package/open-plc-utils/default)
 TITLE:=Qualcomm Atheros Powerline Toolkit
 MENU:=2
endef

# ====================================================================
#
# --------------------------------------------------------------------

define Package/open-plc-utils/description
 This package contains the PLC toolkit.
 It can be used to control PLC devices over the network.
endef

# ====================================================================
#
# --------------------------------------------------------------------

define Build/Compile
	$(TARGET_CONFIG_OPTS) $(MAKE) -C $(PKG_BUILD_DIR) CROSS=$(TARGET_CROSS)
endef

# ====================================================================
# define package program handler;
# --------------------------------------------------------------------

define AddPlcUtil
 define Package/open-plc-utils-$(2)
  $(call Package/open-plc-utils/default)
  TITLE:=$(3)
  DEPENDS+=open-plc-utils
 endef
 define Package/open-plc-utils-$(2)/description
  $(4)
 endef
 define Package/open-plc-utils-$(2)/install
	$(INSTALL_DIR) $$(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/$(1)/$(2) $$(1)/usr/bin
 endef
 $$(eval $$(call BuildPackage,open-plc-utils-$(2)))
endef

# ====================================================================
# call package program handler;
# --------------------------------------------------------------------

$(eval $(call AddPlcUtil,ether,efsu,'Ethernet Frame Send Utility','Used to send Ethernet frames. Ethernet frames can be defined using a text editor.'))
$(eval $(call AddPlcUtil,key,hpavkey,'HomePlugAV Key Generator','Used to generate DAK, NMK and NID keys from Homeplug AV passwords entered on the command line.'))
$(eval $(call AddPlcUtil,key,hpavkeys,'HomePlugAV Key Generator','Used to generate DAK, NMK or NID keys from HomePlug AV passwords stored in a file.'))
$(eval $(call AddPlcUtil,key,mac2pw,'QCA Password Generator','Generate pseudo-random passwords from MAC addresses entered on the command line.'))
$(eval $(call AddPlcUtil,key,mac2pwd,'QCA Password Generator','Generate pseudo-random passwords from an range of MAC addresses stored in a file.'))
$(eval $(call AddPlcUtil,key,rkey,'QCA Random Key Generator','Generate pseudo-random DAK, NMK or NID keys with each invocation of the program.'))
$(eval $(call AddPlcUtil,mdio,mdioblock,'QCA MDIO Block Genertor','Generate a 16-bit MDIO command block from a text file.'))
$(eval $(call AddPlcUtil,mdio,mdioblock2,'QCA MDIO Block Generator','Generate a 32-bit MDIO command block from a text file.'))
$(eval $(call AddPlcUtil,mdio,mdiodump,'QCA MDIO Block Inspector','Display 16-bit or 32-bit MDIO command blocks in human readable format.'))
$(eval $(call AddPlcUtil,mme,hpav,'HomePlugAV Message Monitor','Display incoming HomePlug AV messages frames. Ignore all other frames.'))
$(eval $(call AddPlcUtil,mme,mme,'QCA Message Cheat Sheet','Used for quick lookup of message types and error codes.'))
$(eval $(call AddPlcUtil,nda,int6kecho,'PLC Frame Echo Utility',''))
$(eval $(call AddPlcUtil,nda,plcecho,'PLC Frame Echo Utility','Used to echo a specific frame back from a remote device. The frame is specified as a text file.'))
$(eval $(call AddPlcUtil,nda,plcmod,'PLC Module AddPlcUtil','Used to read and write PLC flash memory modules.'))
$(eval $(call AddPlcUtil,nvm,chknvm,'PLC Firmware File Validator','Performs simple pass/fail sanity check of qca PLC firmware files. Can be used to determine the number and type of images stored within the file."'))
$(eval $(call AddPlcUtil,nvm,chknvm2,'PLC Firmware File Validator','Performs simple pass/fail sanity check of qca PLC firmware files. Can be used to determine the number and type of images stored within the file.'))
$(eval $(call AddPlcUtil,nvm,nvmmerge,'PLC Firmware File Assembler','Used to assemble qca NVM file chains from individual images.'))
$(eval $(call AddPlcUtil,nvm,nvmsplit,'PLC Firmware File Dissector','Explodes a qca NVM file chain into component images.'))
$(eval $(call AddPlcUtil,pib,chkpib,'PLC Parameter File Validator','Performs simple pass/fail sanity check on qca PLC parameter files.'))
$(eval $(call AddPlcUtil,pib,chkpib2,'PLC Parameter File Validator','Performs simple pass/fail sanity check on qca PLC parameter files.'))
$(eval $(call AddPlcUtil,pib,getpib,'QCA PIB Parameter Extractor','Used to extract parameters from a qca PLC parameter file. This qca is of little use without a valid parameter map.'))
$(eval $(call AddPlcUtil,pib,modpib,'QCA PLC Identity AddPlcUtil','Used to configure a small number of common settings stored in qca PLC parameter files. This qca is safer to use than setpib.'))
$(eval $(call AddPlcUtil,pib,pibcomp,'PLC Parameter File Compare','Compare two qca PLC parameter files, field by field. This qca requires an object definition file.'))
$(eval $(call AddPlcUtil,pib,pibdump,'PLC Parameter File Dump','Display the contents of a qca PLC paramenter file, field by field. This qca requies an object definition file.'))
$(eval $(call AddPlcUtil,pib,setpib,'PLC Parameter File Editor','Used to set a variety of PLC parameters. This qca requires detailed knowledge of the file layout which could change from one firmware version to the next.'))
$(eval $(call AddPlcUtil,plc,ampboot,'PLC Boot Utility','Used to manage PLC devices when in bootloader mode. Supports INT6300, INT6400, AR7400 and QCA7450 devices.'))
$(eval $(call AddPlcUtil,plc,amphost,'PLC Host Emulator','Silently waits to service VS_HST_ACTION messages. Supports INT6400 and AR7400 devices.'))
$(eval $(call AddPlcUtil,plc,amprate,'PLC Rate Monitor','Used to sample PHY rates between a device and each neighbor.'))
$(eval $(call AddPlcUtil,plc,amptest,'PLC Test Utility','Used to rung diagnostic applets developed by Qualcomm Atheros engineers. A range of applets are available. Some applets are suitable for production line testing. Suitable for AR7400 and QCA7450 devices.'))
$(eval $(call AddPlcUtil,plc,amptool,'PLC Multi-tool',''))
$(eval $(call AddPlcUtil,plc,ampwait,'PLC Procrastinator','Wait for slected events such as PLC device reset, restart and network accociation. Can be used in test scripts.'))
$(eval $(call AddPlcUtil,plc,int6krule,'PLC QoS Rule AddPlcUtil','Used to set and clear permanent and temporary QoS rules.'))
$(eval $(call AddPlcUtil,plc,plcboot,'PLC Boot Utility','Used to manage PLC devices when in bootloader mode. Supports AR7420, QCA6410 and QCA7000 devices.'))
$(eval $(call AddPlcUtil,plc,plcdevs,'WinPcap Device Enumerator','Enumerate the number and name of WinPcap devices on the local host.'))
$(eval $(call AddPlcUtil,plc,plcfwd,'',''))
$(eval $(call AddPlcUtil,plc,plcget,'PLC Get Property','Get PLC device properties using VS_GET_PROPERTY message.'))
$(eval $(call AddPlcUtil,plc,plchost,'PLC Host Emulator','Silently waits to service VS_HST_ACTION messages. Supports AR7420, QCA6410 and QCA7000 devices.'))
$(eval $(call AddPlcUtil,plc,plchostd,'PLC Host Emulator Daemon','Like plchost but implemented as a true daemon.'))
$(eval $(call AddPlcUtil,plc,plcid,'PLC Key Grabber',''))
$(eval $(call AddPlcUtil,plc,plcrate,'PLC Rate Monitor','Used to sample PHY rates between a device and each neighbor. Can be used to stimulate a mesh network.'))
$(eval $(call AddPlcUtil,plc,plcset,'PLC Set Property','Set PLC device properties using VS_SET_PROPERTY message.'))
$(eval $(call AddPlcUtil,plc,plctest,'PLC Test Utility','Used to run diagnostic applets developed by Qualcomm Atheros engineers. A range of applets are available. Some applets are suitable for production line testing. Suitable for QCA6410, QCA7000 and AR7420 devices.'))
$(eval $(call AddPlcUtil,plc,plctool,'PLC Multi-tool',''))
$(eval $(call AddPlcUtil,programs,netifs,'Ethernet Port Enumerator','Enumerate host Ethernet ports.'))
$(eval $(call AddPlcUtil,programs,plcnets,'PLC Network Enumerator','Enumerate qca PLC networks connected to the host.'))
$(eval $(call AddPlcUtil,serial,ptsctl,'PTS Attenuation Controller',''))
$(eval $(call AddPlcUtil,serial,weeder,'Progammable Attenuator Controller','Set and/or inspect PTS Weeder attenuator module setting from the command line.'))

# ====================================================================
#
# --------------------------------------------------------------------

$(eval $(call BuildPackage,open-plc-utils))

