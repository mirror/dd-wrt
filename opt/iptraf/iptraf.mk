#############################################################
#
# iptraf
#
#############################################################
#
# Created for OpenWRT by
# 			Bruno Bonfils <asyd@debian-fr.org>
# Comments, some cleanup and package support by 
#			Andre Beckedorf <eviljazz@katastrophos.net>


# TARGETS
IPTRAF:=iptraf-2.7.0
IPTRAF_SITE:=ftp://ftp.cebu.mozcom.com/pub/linux/net/
IPTRAF_DIR:=$(BUILD_DIR)/$(IPTRAF)
IPTRAF_SOURCE:=$(IPTRAF).tar.gz
IPTRAF_PATCHSITE:=http://katastrophos.net/wrt54g/sources/
IPTRAF_PATCH:=$(IPTRAF).patch.gz

# Package definitions:
IPTRAF_IPKTARGET:=$(IPTRAF).ipk
IPTRAF_IPKSRC:=$(IPTRAF)-pkg.tar.gz
IPTRAF_IPKSITE:=http://katastrophos.net/wrt54g/sources/

# get the sources...
$(DL_DIR)/$(IPTRAF_SOURCE):
	$(WGET) -P $(DL_DIR) $(IPTRAF_SITE)/$(IPTRAF_SOURCE)

# get the patch...
$(DL_DIR)/$(IPTRAF_PATCH):
	$(WGET) -P $(DL_DIR) $(IPTRAF_PATCHSITE)/$(IPTRAF_PATCH)

# get the packaging rules:
$(DL_DIR)/$(IPTRAF_IPKSRC):
	$(WGET) -P $(DL_DIR) $(IPTRAF_IPKSITE)/$(IPTRAF_IPKSRC)

# unpack the sources...
$(IPTRAF_DIR)/.unpacked: $(DL_DIR)/$(IPTRAF_SOURCE)
	gunzip -c $(DL_DIR)/$(IPTRAF_SOURCE) | tar -C $(BUILD_DIR) -xvf -
	touch  $(IPTRAF_DIR)/.unpacked

# patch the sources...
$(IPTRAF_DIR)/.patched: $(IPTRAF_DIR)/.unpacked $(DL_DIR)/$(IPTRAF_PATCH)
	zcat $(DL_DIR)/$(IPTRAF_PATCH) | patch -d $(IPTRAF_DIR) -p1
	touch $(IPTRAF_DIR)/.patched

# actually compile stuff here...
$(IPTRAF_DIR)/src/iptraf: $(IPTRAF_DIR)/.patched
	(cd $(IPTRAF_DIR)/support ; $(MAKE) CC=$(TARGET_CC) CFLAGS="$(TARGET_CFLAGS)") 
	(cd $(IPTRAF_DIR)/src ; $(MAKE) CC=$(TARGET_CC) CFLAGS="$(TARGET_CFLAGS)") 
	$(STRIP) --strip-unneeded $(IPTRAF_DIR)/src/iptraf
	$(STRIP) --strip-unneeded $(IPTRAF_DIR)/src/rvnamed

# install to $(TARGET_DIR), that is: include it in root file system
$(TARGET_DIR)/usr/sbin/iptraf: $(IPTRAF_DIR)/src/iptraf
	mkdir -p $(TARGET_DIR)/usr/sbin
	cp $(IPTRAF_DIR)/src/iptraf $(TARGET_DIR)/usr/sbin
	cp $(IPTRAF_DIR)/src/rvnamed $(TARGET_DIR)/usr/sbin

# unzip packaging rules...
$(IPTRAF_DIR)/ipkg/rules:  $(DL_DIR)/$(IPTRAF_IPKSRC)
	tar -C $(IPTRAF_DIR) -zxf $(DL_DIR)/$(IPTRAF_IPKSRC)

# build IPK...
$(BUILD_DIR)/$(IPTRAF_IPKTARGET):  $(IPTRAF_DIR)/src/iptraf $(IPTRAF_DIR)/ipkg/rules
	(cd $(IPTRAF_DIR); ipkg-buildpackage )

# INSTRUCTIONS:

# just get sources...
iptraf-source: $(DL_DIR)/$(IPTRAF_SOURCE)

# clean up...
iptraf-clean: 
	-$(MAKE) -C $(IPTRAF_DIR)/support clean
	-$(MAKE) -C $(IPTRAF_DIR)/src clean

# remove the iptraf source directory...
iptraf-dirclean: 
	rm -rf $(IPTRAF_DIR)

# compile iptraf to root file system
iptraf: $(TARGET_DIR)/usr/sbin/iptraf

# compile iptraf to package
iptraf-ipk: $(BUILD_DIR)/$(IPTRAF_IPKTARGET)
